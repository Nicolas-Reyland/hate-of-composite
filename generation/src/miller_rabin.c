#include "miller_rabin.h"

#include <openssl/err.h>

#include "logging.h"
#include "random.h"

unsigned estimate_num_tests(unsigned length)
{
    unsigned num_tests = length;
    if (num_tests < 10)
        num_tests = 10;
    return num_tests;
}

static int generate_prime_candidate(BIGNUM *p, unsigned length);

BIGNUM *miller_rabin_prime_generation(unsigned length, unsigned num_tests)
{
    if (!rng_initialized)
    {
        LOG_WARN("RNG is not initialized... doing it now")
        initialize_rng();
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
    int found_prime;
    int count = 0;
    do
    {
        if (++count % 100 == 0)
            LOG_DEBUG("%d candidates tested", count)

        if (!generate_prime_candidate(p, length))
            goto MillerRabinFailed;

        if ((found_prime = miller_rabin_primality_check(p, num_tests, ctx))
            == -1)
            goto MillerRabinFailed;
    } while (!found_prime);
    LOG_INFO("Found a candidate (%d tries)", count)

    BN_CTX_free(ctx);

    return p;

    /* Something failed */
MillerRabinFailed:
    BN_clear_free(p);

    LOG_DEBUG("Exit with failure")
    return NULL;
}

static int generate_prime_candidate(BIGNUM *p, unsigned length)
{
    // TODO: make this faster (setting bits 8 by 8 or something ?)

    // Fill p at [1:length-2] with random bits
    for (unsigned i = 1; i < length - 1; ++i)
    {
        if (random_decision())
        {
            if (!BN_set_bit(p, i))
            {
                LOG_ERROR("failed to set bit %u", i)
                return 0;
            }
        }
        else
        {
            if (!BN_clear_bit(p, i))
            {
                LOG_ERROR("failed to clear bit %u", i)
                return 0;
            }
        }
    }

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
    BIGNUM *r = BN_CTX_get(ctx);
    BIGNUM *s = BN_CTX_get(ctx);
    BIGNUM *a = BN_CTX_get(ctx);
    BIGNUM *x = BN_CTX_get(ctx);
    BIGNUM *j = BN_CTX_get(ctx);
    BIGNUM *n_minus_one = BN_CTX_get(ctx);
    BIGNUM *bn_one = BN_CTX_get(ctx);
    BIGNUM *bn_two = BN_CTX_get(ctx);
    // Only need to check the last call to BN_CTX_get
    // Src: https://www.openssl.org/docs/manmaster/man3/BN_CTX_get.html
    if (bn_two == NULL)
    {
        unsigned long err_code = ERR_get_error();
        LOG_ERROR("%s", ERR_error_string(err_code, NULL))
        goto MillerRabinTestsEnd;
    }

    BN_one(bn_one);
    BN_set_word(bn_two, 2);
    BN_sub(n_minus_one, n, bn_one);

    /* Miller-Rabin tests */
    result = -2;

    // s = 0
    BN_zero(s);
    // r = n - 1
    BN_copy(r, n_minus_one);

    // n - 1 = r * 2 ^ s
    while (!BN_is_odd(r))
    {
        BN_add(s, s, bn_one);
        BN_div(r, NULL, r, bn_two, ctx);
    }

    for (unsigned i = 0; i < num_tests; ++i)
    {
        // Endpoint is excluded
        random_bn_from_range(a, bn_two, n_minus_one);
        // Modular exponentiation (x = a ^ r % n)
        BN_mod_exp(x, a, r, n, ctx);

        // TODO: check for errors

        //     x != 1     &&        x != n - 1
        if (!BN_is_one(x) && BN_cmp(x, n_minus_one) != 0)
        {
            // j = 1
            BN_one(j);

            //            j < s       &&        x != n - 1
            while (BN_cmp(j, s) < 0 && BN_cmp(x, n_minus_one) != 0)
            {
                // x = x ^ 2 % n
                BN_mod_exp(x, x, bn_two, n, ctx);

                //    x == 1
                if (BN_is_one(x))
                {
                    LOG_DEBUG("found prime with certainty after %u tests", i)
                    result = 1;
                    goto MillerRabinTestsEnd;
                }

                // j += 1
                BN_add_word(j, 1);
            }

            // x != n - 1
            if (BN_cmp(x, n_minus_one) != 0)
            {
                result = 0;
                goto MillerRabinTestsEnd;
            }
        }
    }

    if (result == -2)
        result = 1;

MillerRabinTestsEnd:
    BN_CTX_end(ctx);

    return result;
}
