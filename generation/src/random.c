#include "random.h"

#include <openssl/err.h>
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
    // Fill p at [1:length-2] with random bits
    unsigned pos = 1;
    unsigned until = length - 1;

    while (pos < until)
    {
        int buf = random_int();
        for (size_t i = 0; i < sizeof(int) * 8; ++i)
        {
            int success =
                buf & 1 ? BN_set_bit(p, pos++) : BN_clear_bit(p, pos++);
            if (!success)
            {
                LOG_ERROR("(set/clear _bit) %s",
                          ERR_error_string(ERR_get_error(), NULL))
                return 0;
            }
            if (pos == until)
                break;
            buf >>= 1;
        }
    }

    return 1;
}
