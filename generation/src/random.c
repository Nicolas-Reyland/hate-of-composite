#include "random.h"

#include <time.h>

#include "logging.h"

int rng_initialized = 0;

void initialize_rng(void)
{
    srand(time(NULL));
    rng_initialized = 1;
}

int random_int(void)
{
    return rand();
}

int random_decision()
{
    return random_int() % 2;
}

void random_bn_from_range(BIGNUM *r, BIGNUM *a, BIGNUM *b)
{
    BN_sub(b, b, a);
    BN_rand_range(r, b);
    BN_add(b, b, a);
    BN_add(r, r, a);
}

int generate_prime_candidate(BIGNUM *p, unsigned length)
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
