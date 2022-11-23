#include <openssl/bn.h>
#include <stdlib.h>
#include <string.h>

#include "generate_prime.h"
#include "logging.h"
#include "miller_rabin.h"
#include "primality_test.h"
#include "random.h"

#define CMD_FLAGS_GEN 0b0001
#define CMD_FLAGS_HEX 0b0010
#define CMD_FLAGS_TST 0b0100
#define CMD_FLAGS_ERR 0b1000

static unsigned char parse_args(int argc, char **argv, char *buffer);

static void usage_msg(void);

int main(int argc, char **argv)
{
    char buffer[4096];
    unsigned char flags = parse_args(argc, argv, buffer);

    int exit_code = 0;

    if (flags & CMD_FLAGS_ERR)
    {
        usage_msg();
        exit(EXIT_FAILURE);
    }

    initialize_rng();

    if (flags & CMD_FLAGS_GEN)
    {
        long length = strtol(buffer, NULL, 10);
        BIGNUM *p = generate_prime(length);
        if (p == NULL)
        {
            LOG_ERROR("Failed to generate prime with length %s", buffer);
            exit_code = 1;
        }
        else
        {
            char *p_str = flags & CMD_FLAGS_HEX ? BN_bn2hex(p) : BN_bn2dec(p);
            printf("%s\n", p_str);
            OPENSSL_free(p_str);
            BN_free(p);
            exit_code = 0;
        }
    }
    else if (flags & CMD_FLAGS_TST)
    {
        BIGNUM *n = NULL;
        BN_hex2bn(&n, buffer);
        if (primality_test(n))
        {
            LOG_INFO("%s is a prime number", buffer);
            exit_code = 0;
        }
        else
        {
            LOG_INFO("%s is NOT a prime number", buffer);
            exit_code = 1;
        }
        BN_free(n);
    }
    else
    {
        usage_msg();
        exit(EXIT_SUCCESS);
    }

    // clean up
    CRYPTO_cleanup_all_ex_data();

    return exit_code;
}

static void set_verbosity(char *arg);

static unsigned char parse_args(int argc, char **argv, char *buffer)
{
    unsigned char flags;
    for (int i = 1; i < argc; ++i)
    {
        if (strcmp(argv[i], "--hex") == 0)
        {
            flags |= CMD_FLAGS_HEX;
            continue;
        }
        if (strcmp(argv[i], "-g") == 0)
        {
            flags |= CMD_FLAGS_GEN;
            goto NextArgIsAValue;
        }
        if (strcmp(argv[i], "-t") == 0)
        {
            flags |= CMD_FLAGS_TST;
            goto NextArgIsAValue;
        }
        // verbosity
        if (strncmp(argv[i], "-v", 2) == 0)
        {
            set_verbosity(argv[i]);
            continue;
        }
        flags |= CMD_FLAGS_ERR;
        return flags;

    NextArgIsAValue:
        if (i == argc - 1)
        {
            flags |= CMD_FLAGS_ERR;
            return flags;
        }
        strcpy(buffer, argv[i + 1]);
        ++i;
    }
    if (flags & CMD_FLAGS_GEN && flags & CMD_FLAGS_TST)
        flags |= CMD_FLAGS_ERR;

    return flags;
}

static void set_verbosity(char *arg)
{
    int log_level = 2;
    for (int i = 1; arg[i] == 'v'; ++i)
        ++log_level;

    if (LOG_LEVEL < log_level)
        LOG_LEVEL = log_level;
}

static void usage_msg(void)
{
    fprintf(stderr,
            "usage: ./my_prime [-g [--hex]] [-t number]\n"
            " -g: generate a prime number\n"
            " --hex: print the generated number in hex format\n"
            "\n"
            " -t hex-number: run primality test in the given number\n"
            "     (which should be in hex format)\n");
}
