#include "primality_test.h"

#include "logging.h"
#include "miller_rabin.h"

int primality_test(BIGNUM *p)
{
    unsigned length = BN_num_bits(p);
    unsigned num_tests = estimate_num_tests(length);
    num_tests *= 2;

    BN_CTX *ctx = BN_CTX_secure_new();
    int success = preliminary_checks(p, ctx);
    if (success)
        success = miller_rabin_primality_check(p, num_tests, ctx);
    else
        LOG_DEBUG("Candidate did not pass preliminary checks")
    BN_CTX_free(ctx);

    return success;
}
