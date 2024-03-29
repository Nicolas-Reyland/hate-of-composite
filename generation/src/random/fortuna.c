#include "fortuna.h"

#include <errno.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "random/random.h"
#include "utils/logging.h"
#include "utils/rsa/rsa.h"

#define FORTUNA_NUM_POOLS 3

#ifndef FORTUNA_RESEED_PERIOD
#    define FORTUNA_RESEED_PERIOD 1000
#endif /* !FORTUNA_RESEED_PERIOD  */

int f_seed = 0;
int f_counter = 0;

static void fortuna_seed_from_pool(unsigned pool_index);

int fortuna_seed(void)
{
    if (!initialize_rsa())
        return 0;

    for (unsigned pool_index = 0; pool_index < FORTUNA_NUM_POOLS; ++pool_index)
        fortuna_seed_from_pool(pool_index);
    f_counter = f_seed;
    return 1;
}

void fortuna_cleanup(void)
{
    LOG_DEBUG("Cleaning up Fortuna PRNG")
    cleanup_rsa();
}

int fortuna_rand(void)
{
#ifndef FORTUNA_NO_AUTO_RESEED
    static unsigned num_calls = 0;
    static unsigned pool_counter = 0;
    if (++num_calls == FORTUNA_RESEED_PERIOD)
    {
        num_calls = 0;
        LOG_DEBUG("reseeding with pool counter %u (pool index = %u)",
                  pool_counter, pool_counter % FORTUNA_NUM_POOLS)
        fortuna_seed_from_pool(pool_counter++);
    }
#endif /* !FORTUNA_NO_AUTO_RESEED */

    return rsa_encrypt(f_counter++);
}

void fortuna_seed_from_pool(unsigned pool_index)
{
    pool_index %= FORTUNA_NUM_POOLS;
    switch (pool_index)
    {
    case 0: {
        f_seed = no_init_random_int();
    }
    break;
    case 1: {
        f_seed ^= time(NULL);
    }
    break;
    case 2: {
        f_seed ^= (getpid() << 16) | pthread_self();
    }
    break;
    default:
        LOG_WARN("invalid pool index %d / %d", pool_index, FORTUNA_NUM_POOLS)
        break;
    }
}
