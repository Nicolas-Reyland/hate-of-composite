#include "rsa.h"

#include "utils/logging.h"

typedef unsigned long long ull;

struct rsa_ctx
{
    ull e;
    ull n;
};

struct rsa_ctx rsa_ctx = { 0, 0 };

static ull gcd(ull a, ull h)
{
    ull temp;
    while (1)
    {
        temp = a % h;
        if (temp == 0)
            return h;
        a = h;
        h = temp;
    }
}

void initialize_rsa()
{
    // TODO: use actual random primes
    ull p = 589591; // random_prime();
    ull q = 953399; // random_prime();
    ull n = p * q;
    ull phi = (p - 1) * (q - 1);
    ull e = 2;

    while (e < phi)
    {
        if (gcd(e, phi) == 1)
            break;
        ++e;
    }

    rsa_ctx.n = n;
    rsa_ctx.e = e;

    LOG_DEBUG("Initialized rsa with n = %llu and e = %llu", n, e)
}

static int mod_exp(unsigned x, ull e, ull n);

int rsa_encrypt(int x)
{
    if (rsa_ctx.n == 0)
    {
        LOG_ERROR("RSA was not initialized")
        return 0;
    }
    // encrypted = (x ^ e) % n
    return mod_exp(x, rsa_ctx.e, rsa_ctx.n);
}

int mod_exp(unsigned x, ull e, ull m)
{
    ull x2 = x;
    ull y = 1;
    if (e & 1)
        y = x2;
    while (e)
    {
        e >>= 1;
        x2 = (x2 * x2) % m;
        if (e & 1)
            y = (y * x2) % m;
    }
    return y % m;
}
