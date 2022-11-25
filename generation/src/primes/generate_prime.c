#include "generate_prime.h"

#include "primes/miller_rabin.h"
#include "random/random.h"
#include "utils/logging.h"

BIGNUM *generate_prime(int length)
{
    // Bizarre edge-case
    if (length == 2)
    {
        LOG_INFO("I know you're just trying to do bizarre stuff...")
        BIGNUM *p = BN_secure_new();
        if (p == NULL)
        {
            LOG_ERROR("failed to allocate new BIGNUM for bizarre case: %s",
                      OPENSSL_ERR_STRING)
            return NULL;
        }
        // either 2 or 3
        BN_ULONG word = 2 + (unsigned)random_int() % 2;
        if (!BN_set_word(p, word))
        {
            LOG_ERROR("failed to set word for bizarre case: %s",
                      OPENSSL_ERR_STRING)
            BN_free(p);
            return NULL;
        }
        return p;
    }
    // Real prime generation
    unsigned num_tests = estimate_num_tests(length);
    LOG_INFO("Estimated %d tests for length %d", num_tests, length);
    return miller_rabin_prime_generation(length, num_tests);
}
