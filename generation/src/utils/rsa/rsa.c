#include "rsa.h"

#include <openssl/bn.h>
#include <stddef.h>

#include "primes/generate_prime.h"
#include "utils/logging.h"

#define RSA_PQ_LENGTH 1024

typedef unsigned long long ull;

struct rsa_ctx
{
    BIGNUM *n;
    BIGNUM *e;
    BN_CTX *ctx;
};

struct rsa_ctx rsa_ctx = { .n = NULL, .e = NULL, .ctx = NULL };

int initialize_rsa()
{
    LOG_DEBUG("Initializing RSA")

    BIGNUM *n = NULL;
    BIGNUM *e = NULL;
    BIGNUM *p = NULL;
    BIGNUM *q = NULL;
    BN_CTX *ctx = NULL;

    // Initialize new bignums
    n = BN_secure_new();
    if (n == NULL)
    {
        LOG_ERROR("new bn: n: %s", OPENSSL_ERR_STRING)
        return 0;
    }
    e = BN_secure_new();
    if (e == NULL)
    {
        LOG_ERROR("new bn: e: %s", OPENSSL_ERR_STRING)
        return 0;
    }

    ctx = BN_CTX_secure_new();
    if (ctx == NULL)
    {
        LOG_ERROR("new ctx: %s", OPENSSL_ERR_STRING)
        goto RsaInitializeFailed;
    }
    BN_CTX_start(ctx);

    p = generate_prime(RSA_PQ_LENGTH);
    q = generate_prime(RSA_PQ_LENGTH);
    if (q == NULL || p == NULL)
    {
        LOG_ERROR("generate_prime for p & q: %s", OPENSSL_ERR_STRING)
        goto RsaInitializeFailed;
    }

    // n = p * q
    if (!BN_mul(n, p, q, ctx))
    {
        LOG_ERROR("failed to evaluate 'n = p * q': %s", OPENSSL_ERR_STRING)
        goto RsaInitializeFailed;
    }

    // Initialize variables from ctx
    BIGNUM *p_m_1 = BN_CTX_get(ctx);
    BIGNUM *q_m_1 = BN_CTX_get(ctx);
    BIGNUM *phi = BN_CTX_get(ctx);
    BIGNUM *gcd = BN_CTX_get(ctx);

    if (gcd == NULL)
    {
        LOG_ERROR("failed to init variables from rsa ctx: %s",
                  OPENSSL_ERR_STRING)
        goto RsaInitializeFailed;
    }

    // set 1
    const BIGNUM *one = BN_value_one();

    {
        // calc p - 1
        int err = !BN_sub(p_m_1, p, one);
        // calc q - 1
        err |= !BN_sub(q_m_1, q, one);
        // phi = (p - 1) * (q - 1)
        err |= !BN_mul(phi, p_m_1, q_m_1, ctx);
        // e = 65537
        err |= !BN_set_word(e, 65537);
        if (err)
        {
            LOG_ERROR("setting variables p - 1, q - 1, phi and e failed: %s",
                      OPENSSL_ERR_STRING)
            goto RsaInitializeFailed;
        }
    }

#ifdef BETTER_CHOOSE_RSA_E
    // while e < phi
    while (BN_cmp(e, phi) == -1)
    {
        // gcd = gcd(e, phi)
        if (!BN_gcd(gcd, e, phi, ctx))
        {
            LOG_ERROR("failed to evaluate 'gcd = gcd(e, phi)': %s",
                      OPENSSL_ERR_STRING)
            goto RsaInitializeFailed;
        }
        // if (gcd(e, phi) == 1)
        if (BN_is_one(gcd))
            break;
        // e += 1
        if (!BN_add(e, e, one))
        {
            LOG_ERROR("failed to evaluate 'e = e + 1': %s", OPENSSL_ERR_STRING)
            goto RsaInitializeFailed;
        }
    }
#endif /* BETTER_CHOOSE_RSA_E */

    BN_CTX_end(ctx);
    BN_clear_free(p);
    BN_clear_free(q);

    rsa_ctx.n = n;
    rsa_ctx.e = e;
    rsa_ctx.ctx = ctx;

    return 1;

RsaInitializeFailed:
    BN_CTX_end(ctx);
    BN_CTX_free(ctx);

    BN_clear_free(n);
    BN_clear_free(e);
    BN_clear_free(p);
    BN_clear_free(q);

    return 0;
}

void cleanup_rsa(void)
{
    LOG_DEBUG("Cleaning up RSA block cipher")
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
