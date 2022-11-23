#include <openssl/bn.h>
#include <stdlib.h>
#include <string.h>

#include "generate_prime.h"
#include "logging.h"
#include "miller_rabin.h"
#include "primality_test.h"
#include "random.h"

#define CMD_FLAGS_GEN 0b000001
#define CMD_FLAGS_HEX 0b000010
#define CMD_FLAGS_TST 0b000100
#define CMD_FLAGS_HLP 0b001000
#define CMD_FLAGS_DEC 0b010000
#define CMD_FLAGS_ERR 0b100000

static unsigned parse_args(int argc, char **argv, char *buffer);

static void usage_msg(void);

int main(int argc, char **argv)
{
    char buffer[4096];
    unsigned flags = parse_args(argc, argv, buffer);

    int exit_code = 0;

    if (flags & CMD_FLAGS_ERR || flags & CMD_FLAGS_HLP)
    {
        usage_msg();
        exit(flags & CMD_FLAGS_ERR ? EXIT_FAILURE : EXIT_SUCCESS);
    }

    initialize_prng();

    /* Prime Number Generation */
    if (flags & CMD_FLAGS_GEN)
    {
        char *endptr = NULL;
        long length = strtol(buffer, &endptr, 10);
        if (*endptr != 0)
        {
            LOG_ERROR("Invalid integer: %s (only base allwed is 10)\n", buffer)
            usage_msg();
            exit_code = 2;
        }
        else
        {
            BIGNUM *p = generate_prime(length);
            if (p == NULL)
            {
                LOG_ERROR("Failed to generate prime with length %s", buffer);
                exit_code = 1;
            }
            else
            {
                char *p_str =
                    flags & CMD_FLAGS_HEX ? BN_bn2hex(p) : BN_bn2dec(p);
                printf("%s\n", p_str);
                OPENSSL_free(p_str);
                BN_free(p);
                exit_code = 0;
            }
        }
    }
    /* Primality Testing */
    else if (flags & CMD_FLAGS_TST)
    {
        BIGNUM *n = NULL;
        int (*bn_read_fn)(BIGNUM * *a, const char *str);

        if (flags & CMD_FLAGS_DEC)
        {
            LOG_DEBUG("Reading %s as decimal value", buffer);
            bn_read_fn = BN_dec2bn;
        }
        else
        {
            LOG_DEBUG("Reading %s as hex value (default)", buffer);
            bn_read_fn = BN_hex2bn;
        }
        size_t num_read = (*bn_read_fn)(&n, buffer);
        size_t num_chars_to_read = strlen(buffer);
        if (num_read == 0 || num_read < num_chars_to_read)
        {
            LOG_ERROR("Could not read given prime number \"%s\"", buffer)
            exit_code = 2;
        }
        else
        {
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
    }
    /* No command input */
    else
    {
        usage_msg();
        exit_code = 0;
    }

    // clean up
    CRYPTO_cleanup_all_ex_data();

    return exit_code;
}

static void set_verbosity(char *arg);

static unsigned parse_args(int argc, char **argv, char *buffer)
{
    unsigned flags = 0;
    for (int i = 1; i < argc; ++i)
    {
        if (strcmp(argv[i], "--hex") == 0)
        {
            flags |= CMD_FLAGS_HEX;
            continue;
        }
        if (strcmp(argv[i], "--dec") == 0)
        {
            flags |= CMD_FLAGS_DEC;
            continue;
        }
        if (strcmp(argv[i], "-g") == 0)
        {
            if (flags & (CMD_FLAGS_GEN | CMD_FLAGS_TST))
                return CMD_FLAGS_ERR;
            flags |= CMD_FLAGS_GEN;
            goto NextArgIsAValue;
        }
        if (strcmp(argv[i], "-t") == 0)
        {
            if (flags & (CMD_FLAGS_GEN | CMD_FLAGS_TST))
                return CMD_FLAGS_ERR;
            flags |= CMD_FLAGS_TST;
            goto NextArgIsAValue;
        }
        // help
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0)
            return CMD_FLAGS_HLP;
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
        "usage: ./my_prime [-h] [--help] [-g length] [-t number] [--hex] "
        "[--dec] [-v] [--verbose] [-vv] [--debug]\n"
        "  -h | --help: show this help message\n"
        "\n"
        " -g length: generate a prime number of `length` bits (generated >= "
        "2^length)\n"
        "\n"
        " -t hex-number: run primality test in the given number\n"
        "     (which should be in hex format)\n"
        "\n"
        "  -v | --verbose: log info messages\n"
        " -vv | --debug: log info and debug messages\n"
        "\n"
        " --hex: for -g only. print the generated number in hex format\n"
        " --dec: for -t only. accept input string as decimal, instead of "
        "hex\n");
}
