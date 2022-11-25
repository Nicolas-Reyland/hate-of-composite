#include "primality_test.h"

#include "primes/miller_rabin.h"
#include "primes/preliminary.h"
#include "utils/logging.h"

int primality_test(BIGNUM *p, unsigned num_tests, BN_CTX *ctx)
{
    int success = preliminary_checks(p, ctx);
    if (success == 1)
        success = miller_rabin_primality_check(p, num_tests, ctx);

    return success;
}

int primality_test_once(BIGNUM *p)
{
    unsigned length = BN_num_bits(p);
    unsigned num_tests = estimate_num_tests(length);

    BN_CTX *ctx = BN_CTX_secure_new();
    int success = primality_test(p, num_tests, ctx);
    BN_CTX_free(ctx);

    return success;
}
