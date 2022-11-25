#ifndef PRIMALITY_TEST_H
#define PRIMALITY_TEST_H

#include <openssl/bn.h>

int primality_test(BIGNUM *p, unsigned num_tests, BN_CTX *ctx);

int primality_test_once(BIGNUM *p);

#endif /* !PRIMALITY_TEST_H */
