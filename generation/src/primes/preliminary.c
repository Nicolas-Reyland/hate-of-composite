#include "preliminary.h"

#include <errno.h>
#include <string.h>

#include "utils/logging.h"

// 24 first prime numbers
static BN_ULONG PRELIMINARY_PRIMES[] = { 3,  5,  7,  11, 13, 17, 19, 23,
                                         29, 31, 37, 41, 43, 47, 53, 59,
                                         61, 67, 71, 73, 79, 83, 89, 97 };

static size_t NUM_PRELIMINARY_PRIMES =
    sizeof(PRELIMINARY_PRIMES) / sizeof(PRELIMINARY_PRIMES[0]);

static BIGNUM **PRELIMINARY_PRIMES_BIGNUMS = NULL;

int setup_preliminary(void)
{
    if (PRELIMINARY_PRIMES_BIGNUMS != NULL)
    {
        LOG_WARN("Preliminary already initialized")
        return -1;
    }

    PRELIMINARY_PRIMES_BIGNUMS = calloc(NUM_PRELIMINARY_PRIMES, sizeof(void *));
    if (PRELIMINARY_PRIMES_BIGNUMS == NULL)
    {
        LOG_ERROR("failed to allocate PPB: %s", strerror(errno))
        return 0;
    }

    for (size_t i = 0; i < NUM_PRELIMINARY_PRIMES; ++i)
    {
        BN_ULONG small_prime_word = PRELIMINARY_PRIMES[i];
        BIGNUM *small_prime = BN_new();
        if (small_prime == NULL)
        {
            LOG_ERROR("failed to allocate PPB[%zu] %lu: %s", i,
                      small_prime_word, OPENSSL_ERR_STRING)
            goto SetupPreliminaryFailed;
        }
        if (!BN_set_word(small_prime, small_prime_word))
        {
            LOG_ERROR("failed to set word of PPB[%zu] to %lu: %s", i,
                      small_prime_word, OPENSSL_ERR_STRING)
            goto SetupPreliminaryFailed;
        }
        PRELIMINARY_PRIMES_BIGNUMS[i] = small_prime;
    }

    return 1;

SetupPreliminaryFailed:

    cleanup_preliminary();
    return 0;
}

void cleanup_preliminary()
{
    if (PRELIMINARY_PRIMES_BIGNUMS == NULL)
    {
        LOG_DEBUG("preliminary primes bignums is NULL")
        return;
    }

    for (size_t i = 0; i < NUM_PRELIMINARY_PRIMES; ++i)
        BN_free(PRELIMINARY_PRIMES_BIGNUMS[i]);

    free(PRELIMINARY_PRIMES_BIGNUMS);
    PRELIMINARY_PRIMES_BIGNUMS = NULL;
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
    if (ctx == NULL)
    {
        LOG_ERROR("ctx is NULL")
        return -1;
    }

    if (PRELIMINARY_PRIMES_BIGNUMS == NULL)
    {
        LOG_ERROR("PPB is NULL")
        return -1;
    }

    /* Trivial- and Edge-cases */
    if (BN_is_word(n, 2))
        return 1;
    if (BN_is_negative(n))
    {
        LOG_WARN("Negative prime candidate")
        return 0;
    }
    if (!BN_is_odd(n) || BN_is_zero(n) || BN_is_one(n))
        return 0;

    const size_t num_preliminary_primes =
        sizeof(PRELIMINARY_PRIMES) / sizeof(PRELIMINARY_PRIMES[0]);

    BN_CTX_start(ctx);

    BIGNUM *rem = BN_CTX_get(ctx);
    if (rem == NULL)
    {
        LOG_ERROR("BN_CTX_get failed: %s", OPENSSL_ERR_STRING)
        BN_CTX_end(ctx);
        return -1;
    }

    for (size_t i = 0; i < num_preliminary_primes; ++i)
    {
        BIGNUM *div = PRELIMINARY_PRIMES_BIGNUMS[i];
        if (!BN_mod(rem, n, div, ctx))
        {
            LOG_ERROR("failed to calculate mod %lu", PRELIMINARY_PRIMES[i])
            BN_CTX_end(ctx);
            return -1;
        }
        if (BN_is_zero(rem))
        {
            BN_CTX_end(ctx);
            return 0;
        }
    }

    BN_CTX_end(ctx);
    return 1;
}
