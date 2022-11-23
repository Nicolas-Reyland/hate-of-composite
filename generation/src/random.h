#ifndef RANDOM_H
#define RANDOM_H

#include <openssl/bn.h>

extern int prng_initialized;

void initialize_prng(void);

int random_int(void);

int random_decision(void);

void random_bn_from_range(BIGNUM *r, BIGNUM *a, BIGNUM *b);

int generate_prime_candidate(BIGNUM *p, unsigned length);

#endif /* !RANDOM_H */
