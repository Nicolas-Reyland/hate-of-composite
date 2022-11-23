#include "primality_test.h"

#include "miller_rabin.h"

int primality_test(BIGNUM *p)
{
    unsigned num_tests = 1000;
    BN_CTX *ctx = BN_CTX_secure_new();
    int success = miller_rabin_primality_check(p, num_tests, ctx);
    BN_CTX_free(ctx);

    return success;
}
