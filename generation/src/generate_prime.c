#include "generate_prime.h"

#include "miller_rabin.h"

BIGNUM *generate_prime(int length)
{
    unsigned num_tests = estimate_num_tests(length);
    return miller_rabin_prime_generation(length, num_tests);
}
