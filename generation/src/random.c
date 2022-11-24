#include "random.h"

#include <errno.h>
#include <openssl/err.h>
#include <pthread.h>
#include <string.h>
#include <sys/random.h>
#include <time.h>
#include <unistd.h>

#include "logging.h"

int prng_initialized = 0;

void initialize_prng(void)
{
    int seed;
    if (getrandom(&seed, sizeof(int), GRND_RANDOM) == -1)
    {
        LOG_WARN("could not get random data from `getrandom`: %s",
                 strerror(errno))
    }
    seed ^= time(NULL);
    seed ^= pthread_self();
    seed ^= getpid() << 20;

    srand(seed);
    prng_initialized = 1;
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
    // TODO: fix this too (using own fn for random bn generation)

    BN_sub(b, b, a);
    BN_rand_range(r, b);
    BN_add(b, b, a);
    BN_add(r, r, a);
}

static int random_bn_fill(BIGNUM *p, unsigned length, unsigned pos,
                          unsigned until);

int bn_rand_range(BIGNUM *n, BIGNUM *range)
{
    return 0;
}

int generate_prime_candidate(BIGNUM *p, unsigned length)
{
    // Fill p at [1:length-2] with random bits
    return random_bn_fill(p, length, 1, length - 1);
}

int random_bn_fill(BIGNUM *p, unsigned length, unsigned pos, unsigned until)
{
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
