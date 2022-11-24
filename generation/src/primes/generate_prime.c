#include "generate_prime.h"

#include "primes/miller_rabin.h"
#include "utils/logging.h"

BIGNUM *generate_prime(int length)
{
    unsigned num_tests = estimate_num_tests(length);
    LOG_INFO("Estimated %d tests for length %d", num_tests, length);
    return miller_rabin_prime_generation(length, num_tests);
}
