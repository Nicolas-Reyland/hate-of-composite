#include "random.h"

#include <errno.h>
#include <openssl/err.h>
#include <string.h>
#include <sys/random.h>

#include "random/fortuna.h"
#include "utils/logging.h"

int prng_initialized = 0;

int initialize_prng(void)
{
    if (!fortuna_seed())
    {
        LOG_INFO("PRNG initialization failed")
        return 0;
    }

    LOG_INFO("PRNG initialization succeeded")
    prng_initialized = 1;
    return 1;
}

void cleanup_prng(void)
{
    if (prng_initialized)
    {
        LOG_DEBUG("Cleaning up PRNG")
        fortuna_cleanup();
        prng_initialized = 0;
    }
    else
        LOG_WARN("No need to clean up prng: not intiialized")
}

int random_int(void)
{
    return prng_initialized ? fortuna_rand() : no_init_random_int();
}

int no_init_random_int(void)
{
    int value;
    if (getrandom(&value, sizeof(int), GRND_RANDOM) == -1)
        LOG_ERROR("%s", strerror(errno))
    return value;
}

int random_decision()
{
    return random_int() % 2;
}

void random_bn_from_range(BIGNUM *r, BIGNUM *a, BIGNUM *b)
{
    BN_sub(b, b, a);
    bn_rand_max(r, b);
    BN_add(b, b, a);
    BN_add(r, r, a);
}

static int random_bn_fill(BIGNUM *p, unsigned pos, unsigned until,
                          int (*rng_f)(void));

void bn_rand_max(BIGNUM *n, BIGNUM *max)
{
    unsigned length = BN_num_bits(max);
    BN_set_bit(n, length - 1);
    random_bn_fill(n, 0, length, &random_int);

    //        n >= max
    while (BN_cmp(n, max) != -1)
    {
        BN_lshift(n, n, 1);
        BN_clear_bit(n, length);
        if (random_decision())
            BN_set_bit(n, 0);
    }
}

int generate_prime_candidate(BIGNUM *p, unsigned length)
{
    // Fill p at [1:length-2] with random bits
    int (*rng_f)(void) = NULL;
    if (prng_initialized)
        rng_f = &random_int;
    else
        rng_f = &no_init_random_int;
    return random_bn_fill(p, 1, length - 1, rng_f);
}

int random_bn_fill(BIGNUM *p, unsigned pos, unsigned until, int (*rng_f)(void))
{
    while (pos < until)
    {
        int buf = (*rng_f)();
        for (size_t i = 0; i < sizeof(int) * 8; ++i)
        {
            int success =
                buf & 1 ? BN_set_bit(p, pos++) : BN_clear_bit(p, pos++);
            if (!success)
            {
                LOG_ERROR("(set/clear _bit) %s", OPENSSL_ERR_STRING)
                return 0;
            }
            if (pos == until)
                break;
            buf >>= 1;
        }
    }

    return 1;
}
