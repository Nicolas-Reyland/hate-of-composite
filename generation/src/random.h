#ifndef RANDOM_H
#define RANDOM_H

#include <openssl/bn.h>

extern int rng_initialized;

void initialize_rng(void);

int random_int(void);

int random_decision(void);

void random_bn_from_range(BIGNUM *r, BIGNUM *a, BIGNUM *b);

#endif /* !RANDOM_H */
