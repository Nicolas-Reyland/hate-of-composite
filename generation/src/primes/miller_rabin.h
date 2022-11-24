#ifndef MILLER_RABIN_H
#define MILLER_RABIN_H

#include <openssl/bn.h>

/*
 * Generate a pseudo-prime number using the miller rabin
 * algorithm.
 */
BIGNUM *miller_rabin_prime_generation(unsigned length, unsigned num_tests);

int miller_rabin_primality_check(BIGNUM *n, unsigned num_tests, BN_CTX *ctx);

unsigned estimate_num_tests(unsigned length);

int preliminary_checks(BIGNUM *n, BN_CTX *ctx);

#endif /* !MILLER_RABIN_H */
