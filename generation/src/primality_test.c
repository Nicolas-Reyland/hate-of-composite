#include "primality_test.h"

#include "miller_rabin.h"

int primality_test(BIGNUM *p)
{
    unsigned length = BN_num_bits(p);
    unsigned num_tests = estimate_num_tests(length);

    BN_CTX *ctx = BN_CTX_secure_new();
    int success = miller_rabin_primality_check(p, num_tests, ctx);
    BN_CTX_free(ctx);

    return success;
}
