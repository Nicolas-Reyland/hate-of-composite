#ifndef PRIMALITY_TEST_H
#define PRIMALITY_TEST_H

#include <openssl/bn.h>

int primality_test(BIGNUM *p);

#endif /* !PRIMALITY_TEST_H */
