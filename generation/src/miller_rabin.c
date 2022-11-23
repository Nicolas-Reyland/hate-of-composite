#include "miller_rabin.h"

#include <math.h>
#include <openssl/err.h>

#include "logging.h"
#include "random.h"

// 24 first prime numbers
static BN_ULONG PRELIMINARY_PRIMES[] = { 3,  5,  7,  11, 13, 17, 19, 23,
                                         29, 31, 37, 41, 43, 47, 53, 59,
                                         61, 67, 71, 73, 79, 83, 89, 97 };

/*
 * Src:
 * https://security.stackexchange.com/questions/4544/how-many-iterations-of-rabin-miller-should-be-used-to-generate-cryptographic-saf
 *
 * (Post by Thomas Pornin:
 * https://security.stackexchange.com/users/655/thomas-pornin)
 */
unsigned estimate_num_tests(unsigned length)
{
    unsigned num_tests = length;
    if (num_tests < 10)
        num_tests = 10;
    if (num_tests > 40)
        num_tests = 40;

    return num_tests;
}

BIGNUM *miller_rabin_prime_generation(unsigned length, unsigned num_tests)
{
    if (!prng_initialized)
    {
        LOG_WARN("RNG is not initialized... doing it now")
        initialize_prng();
    }

    /* Validate arguments */
    if (length < 2)
    {
        LOG_ERROR("Invalid int length: %u", length)
        return NULL;
    }

    /* Initialize big number */
    BIGNUM *p = BN_secure_new();
    if (p == NULL)
    {
        LOG_ERROR("Out of memory")
        return NULL;
    }
    /* Initialize default bit values */
    if (!BN_set_bit(p, 0) || !BN_set_bit(p, length - 1))
    {
        LOG_ERROR("Failed to initialize candidate first and/or last bits")
        return NULL;
    }

    /* Initialize BN Context */
    BN_CTX *ctx = BN_CTX_secure_new();
    if (ctx == NULL)
    {
        LOG_ERROR("Out of memory")
        goto MillerRabinFailed;
    }

    /* Find a prime number (trial and error) */
    int found_prime = 0, count = 0;
    do
    {
        if (++count % 100 == 0)
            LOG_DEBUG("%d candidates tested", count)

        if (!generate_prime_candidate(p, length))
            goto MillerRabinFailed;

        if (!preliminary_checks(p, ctx))
            continue;

#if 0
        // if debugging is set
        if (LOG_LEVEL >= 4)
        {
            char *buf = BN_bn2dec(p);
            LOG_DEBUG("%s passed preliminary", buf);
            OPENSSL_free(buf);
        }
#endif /* 0 */

        if ((found_prime = miller_rabin_primality_check(p, num_tests, ctx))
            == -1)
            goto MillerRabinFailed;
    } while (!found_prime);
    LOG_INFO("Found a candidate (%d tries)", count)

    BN_CTX_free(ctx);

    return p;

    /* Something failed */
MillerRabinFailed:
    BN_CTX_free(ctx);
    BN_clear_free(p);

    LOG_DEBUG("Exit with failure")
    return NULL;
}

/*
 *
 *
 * Src:
 * https://security.stackexchange.com/questions/4544/how-many-iterations-of-rabin-miller-should-be-used-to-generate-cryptographic-saf
 * (Post by JAred Deckard:
 * https://security.stackexchange.com/users/160084/jared-deckard)
 */
int preliminary_checks(BIGNUM *n, BN_CTX *ctx)
{
    // TODO: check for errors

    const size_t num_preliminary_primes =
        sizeof(PRELIMINARY_PRIMES) / sizeof(PRELIMINARY_PRIMES[0]);

    BN_CTX_start(ctx);

    BIGNUM *rem = BN_CTX_get(ctx);
    BIGNUM *div = BN_CTX_get(ctx);

    for (size_t i = 0; i < num_preliminary_primes; ++i)
    {
        BN_set_word(div, PRELIMINARY_PRIMES[i]);
        BN_mod(rem, n, div, ctx);
        if (BN_is_zero(rem))
        {
            BN_CTX_end(ctx);
            return 0;
        }
    }

    BN_CTX_end(ctx);
    return 1;
}

int miller_rabin_primality_check(BIGNUM *n, unsigned num_tests, BN_CTX *ctx)
{
    /* Trivial- and Edge-cases */
    if (BN_is_word(n, 2))
        return 1;
    if (!BN_is_odd(n) || BN_is_zero(n) || BN_is_one(n))
        return 0;
    if (BN_is_negative(n))
    {
        LOG_WARN("Negative prime candidate")
        return -1;
    }

    int result = -1;

    /* BIGNUM Initializations */
    BN_CTX_start(ctx);
    unsigned s;
    BIGNUM *d = BN_CTX_get(ctx);
    BIGNUM *a = BN_CTX_get(ctx);
    BIGNUM *x = BN_CTX_get(ctx);
    BIGNUM *y = BN_CTX_get(ctx);
    BIGNUM *n_minus_one = BN_CTX_get(ctx);
    BIGNUM *one = BN_CTX_get(ctx);
    BIGNUM *two = BN_CTX_get(ctx);
    // Only need to check the last call to BN_CTX_get
    // Src: https://www.openssl.org/docs/manmaster/man3/BN_CTX_get.html
    if (two == NULL)
    {
        unsigned long err_code = ERR_get_error();
        LOG_ERROR("%s", ERR_error_string(err_code, NULL))
        goto MillerRabinTestsEnd;
    }

    // TODO: check for errors

    BN_one(one);
    BN_set_word(two, 2);
    BN_sub(n_minus_one, n, one);

    /* Miller-Rabin tests */
    result = -2;

    // s = 0
    s = 0;
    // d = n - 1
    BN_copy(d, n_minus_one);

    // while r is even ...
    while (!BN_is_odd(d))
    {
        // s += 1
        ++s;
        // d /= 2
        BN_rshift1(d, d);
    }
    // d * 2^s = n - 1

    for (unsigned round = 0; round < num_tests; ++round)
    {
        // Endpoint is excluded
        random_bn_from_range(a, two, n_minus_one);
        // Modular exponentiation (x = a ^ d % n)
        BN_mod_exp(x, a, d, n, ctx);

        for (unsigned sub_round = 0; sub_round < s; ++sub_round)
        {
            // y = x^2 % n
            BN_mod_exp(y, x, two, n, ctx);

            //     y == 1    &&    x != 1     &&     X != n - 1
            if (BN_is_one(y) && !BN_is_one(x) && BN_cmp(x, n_minus_one) != 0)
            {
                // nontrivial square root of 1 modulo n
                result = 0;
                goto MillerRabinTestsEnd;
            }
            // x = y
            BN_copy(x, y);
        }

        //    y != 1
        if (!BN_is_one(y))
        {
            result = 0;
            goto MillerRabinTestsEnd;
        }
    }

    if (result == -2)
        result = 1;

MillerRabinTestsEnd:
    BN_CTX_end(ctx);

    return result;
}
