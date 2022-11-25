#ifndef PRELIMINARY_H
#define PRELIMINARY_H

#include <openssl/bn.h>

int setup_preliminary(void);

void cleanup_preliminary(void);

int preliminary_checks(BIGNUM *n, BN_CTX *ctx);

#endif /* !PRELIMINARY_H */
