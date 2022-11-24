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
#define FORTUNA_RESEED_PERIOD 1000

int f_seed = 0;
int f_counter = 0;

static void fortuna_seed_from_pool(unsigned pool_index);

void fortuna_seed(void)
{
    initialize_rsa();
    for (unsigned pool_index = 0; pool_index < FORTUNA_NUM_POOLS; ++pool_index)
        fortuna_seed_from_pool(pool_index);
    f_counter = f_seed;
}

int fortuna_rand(void)
{
#ifdef FORTUNA_AUTO_RESEED
    static unsigned num_calls = 0;
    static unsigned pool_index = 0;
    if (++num_calls == FORTUNA_RESEED_PERIOD)
    {
        num_calls = 0;
        LOG_DEBUG("reseeding random")
        fortuna_seed_from_pool(pool_index++);
    }
#endif /* !FORTUNA_AUTO_RESEED */

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
