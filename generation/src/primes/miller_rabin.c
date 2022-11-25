#include "miller_rabin.h"

#include <math.h>
#include <openssl/err.h>

#include "primes/primality_test.h"
#include "random/random.h"
#include "utils/logging.h"

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
    /* Validate arguments */
    if (length < 2)
    {
        LOG_ERROR("Invalid length: %u", length)
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
    int count = 0, success = 0;
    do
    {
        if (++count % 100 == 0)
            LOG_DEBUG("%d candidates tested", count)

        if (!generate_prime_candidate(p, length))
            goto MillerRabinFailed;

        if ((success = primality_test(p, num_tests, ctx)) == -1)
            goto MillerRabinFailed;
    } while (!success);
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

int miller_rabin_primality_check(BIGNUM *n, unsigned num_tests, BN_CTX *ctx)
{
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
        LOG_ERROR("initializing constants from ctx: %s", OPENSSL_ERR_STRING)
        goto MillerRabinTestsEnd;
    }

    if (!BN_one(one) || !BN_set_word(two, 2) || !BN_sub(n_minus_one, n, one))
    {
        LOG_ERROR("pre-setting constants: %s", OPENSSL_ERR_STRING)
        goto MillerRabinTestsEnd;
    }

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
