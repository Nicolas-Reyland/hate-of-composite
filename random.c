#include "random.h"

#include <time.h>

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
    (void)a;
    BN_rand_range(r, b);
}
