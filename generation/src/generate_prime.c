#include "generate_prime.h"

#include "miller_rabin.h"

BIGNUM *generate_prime(int length)
{
    unsigned num_tests = 1000;
    return miller_rabin_prime_generation(length, num_tests);
}
