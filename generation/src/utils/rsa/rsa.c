#include "rsa.h"

#include <openssl/bn.h>
#include <stddef.h>

#include "utils/logging.h"

#define RSA_PQ_PRIME_SET_SIZE 1000000

typedef unsigned long long ull;

struct rsa_ctx
{
    BIGNUM *n;
    BIGNUM *e;
    BN_CTX *ctx;
};

struct rsa_ctx rsa_ctx = { .n = NULL, .e = NULL, .ctx = NULL };

void initialize_rsa()
{
    // TODO: check for errors
    LOG_DEBUG("Initializing RSA")

    BIGNUM *n = BN_new();
    BIGNUM *e = BN_new();

    BN_CTX *ctx = BN_CTX_secure_new();
    BN_CTX_start(ctx);

    BIGNUM *p = BN_CTX_get(ctx);
    BN_set_word(p, 589591);
    BIGNUM *q = BN_CTX_get(ctx);
    BN_set_word(q, 953399);

    // n = p * q
    BN_mul(n, p, q, ctx);

    // set 1
    const BIGNUM *one = BN_value_one();

    // calc p - 1
    BIGNUM *p_m_1 = BN_CTX_get(ctx);
    BN_sub(p_m_1, p, one);

    // calc q - 1
    BIGNUM *q_m_1 = BN_CTX_get(ctx);
    BN_sub(q_m_1, q, one);

    // phi = (p - 1) * (q - 1)
    BIGNUM *phi = BN_CTX_get(ctx);
    BN_mul(phi, p_m_1, q_m_1, ctx);

    // e = 2
    BN_set_word(e, 2);

    // gcd tmp variable
    BIGNUM *gcd = BN_CTX_get(ctx);

    // while e < phi
    while (BN_cmp(e, phi) == -1)
    {
        BN_gcd(gcd, e, phi, ctx);
        // if (gcd(e, phi) == 1)
        if (BN_is_one(gcd))
            break;
        // e += 1
        BN_add(e, e, one);
    }

    BN_CTX_end(ctx);

    rsa_ctx.n = n;
    rsa_ctx.e = e;
    // TODO: maybe clear and re-allocate a new ctx ?
    // We used a lot of variables, maybe there is some unecessary waste of
    // memory ?
    // + values such as p and q should not remain in memory, for security
    // reasons ?
    rsa_ctx.ctx = ctx;
}

void cleanup_rsa(void)
{
    LOG_DEBUG("Cleaning up rsa")
    BN_clear_free(rsa_ctx.n);
    BN_clear_free(rsa_ctx.e);
    BN_CTX_free(rsa_ctx.ctx);
}

int rsa_encrypt(int x)
{
    if (rsa_ctx.n == NULL)
    {
        LOG_ERROR("RSA was not initialized")
        return 0;
    }

    const int int_num_bytes = sizeof(int);

    BN_CTX_start(rsa_ctx.ctx);

    BIGNUM *bn_x = BN_CTX_get(rsa_ctx.ctx);
    BN_set_word(bn_x, x);
    BIGNUM *bn_y = BN_CTX_get(rsa_ctx.ctx);
    // y = (x ^ e) % n
    BN_mod_exp(bn_y, bn_x, rsa_ctx.e, rsa_ctx.n, rsa_ctx.ctx);
    // truncate bn_y if needed
    if (BN_num_bytes(bn_y) > int_num_bytes)
        BN_mask_bits(bn_y, sizeof(int) * 8);
    int y = BN_get_word(bn_y);

    BN_CTX_end(rsa_ctx.ctx);

    return y;
}
