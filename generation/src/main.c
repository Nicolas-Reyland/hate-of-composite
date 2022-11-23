#include <openssl/bn.h>
#include <stdlib.h>
#include <string.h>

#include "generate_prime.h"
#include "logging.h"
#include "miller_rabin.h"
#include "primality_test.h"
#include "random.h"

#define CMD_FLAGS_GEN 0b00001
#define CMD_FLAGS_HEX 0b00010
#define CMD_FLAGS_TST 0b00100
#define CMD_FLAGS_HLP 0b01000
#define CMD_FLAGS_ERR 0b10000

static unsigned char parse_args(int argc, char **argv, char *buffer);

static void usage_msg(void);

int main(int argc, char **argv)
{
    char buffer[4096];
    unsigned char flags = parse_args(argc, argv, buffer);

    int exit_code = 0;

    if (flags & CMD_FLAGS_ERR || flags & CMD_FLAGS_HLP)
    {
        usage_msg();
        exit(flags & CMD_FLAGS_ERR ? EXIT_FAILURE : EXIT_SUCCESS);
    }

    initialize_prng();

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
        int success = primality_test(n);
        switch (success)
        {
        case 1: {
            LOG_INFO("%s is a prime number", buffer);
            exit_code = 1;
        }
        break;
        case 0: {
            LOG_INFO("%s is NOT a prime number", buffer);
            exit_code = 0;
        }
        break;
        default: {
            LOG_WARN("primality check exited with a failure status for %s",
                     buffer)
            exit_code = 0;
        }
        break;
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
    unsigned char flags = 0;
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
        // help
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0)
        {
            flags |= CMD_FLAGS_HLP;
            return flags;
        }
        // verbosity
        if (strncmp(argv[i], "-v", 2) == 0)
        {
            set_verbosity(argv[i]);
            continue;
        }
        if (strcmp(argv[i], "--verbose") == 0)
        {
            set_verbosity("-v");
            continue;
        }
        if (strcmp(argv[i], "--debug") == 0)
        {
            set_verbosity("-vv");
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
    fprintf(
        stderr,
        "usage: ./my_prime [-h] [--help] [-g length [--hex]] [-t number] [-v] "
        "[--verbose] [-vv] [--debug]\n"
        "  -h | --help: show this help message\n"
        "\n"
        " -g length: generate a prime number of `length` bits (generated >= "
        "2^length)\n"
        " --hex: print the generated number in hex format\n"
        "\n"
        " -t hex-number: run primality test in the given number\n"
        "     (which should be in hex format)\n"
        "\n"
        "  -v | --verbose: log info messages\n"
        " -vv | --debug: log info and debug messages\n"
        "\n");
}
