#ifndef MILLER_RABIN_H
#define MILLER_RABIN_H

#include <openssl/bn.h>

/*
 * Generate a pseudo-prime number using the miller rabin
 * algorithm.
 */
BIGNUM *miller_rabin_prime_generation(unsigned length, unsigned num_tests);

#endif /* !MILLER_RABIN_H */
