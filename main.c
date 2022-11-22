#include <openssl/bn.h>

#include "miller_rabin.h"
#include "random.h"

int main(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    initialize_rng();

    BIGNUM *p = miller_rabin_prime_generation(100, 128);

    if (p == NULL)
    {
        fprintf(stderr, "No prime number found :(\n");
        return 1;
    }

    // BN_bn2hex for hexadecimal
    char *p_str = BN_bn2dec(p);
    printf("%s\n", p_str);

    // clean up
    OPENSSL_free(p_str);
    BN_free(p);
    CRYPTO_cleanup_all_ex_data();

    return 0;
}
