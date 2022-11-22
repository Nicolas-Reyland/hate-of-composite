#!/usr/bin/env python3
import sys
import random


def is_prime(n: int, k: int = 128) -> bool:
    """Test if a number is prime

    Args:
        n (int) : prime number candidate
        k (int) :
            number of tests to do.
            The bigger the nnumber of tests, the higher the probability that `n`
            is actually a prime number (and not a *liar*)

    Returns prime status of number
    """

    # First, eliminate trivial- and edge-cases
    if n in [2, 3]:
        return True
    if n < 2 or n % 2 == 0:
        return False

    s = 0
    r = n - 1
    # while r is odd ...
    while r & 1 == 0:
        s += 1
        r //= 2
    # do the k tests
    for _ in range(k):
        # endpoint is excluded
        a = random.randrange(2, n - 1)
        # modular exponentiation is built-in in python
        x = pow(a, r, n)

        if x != 1 and x != n - 1:
            j = 1
            while j < s and x != n - 1:
                x = pow(x, 2, n)
                if x == 1:
                    return False
                j += 1
            if x != n - 1:
                return False

    return True


def generate_prime_candidate(num_bits: int) -> int:
    """Generate an odd integer `n` such as `n` >= 2^`num_bits`

    Args:
        num_bits (int) : number of bits in the generated integer

    Exceptions:
        ValueError : Number of bits is less than 2
    """
    if num_bits < 1:
        raise ValueError(f"Number of bits {num_bits} < 2")

    c: int = random.getrandbits(num_bits)
    c |= (1 << num_bits - 1) | 1
    return c


def generate_prime(num_bits: int) -> int:
    """Generate a prime number `n` such as `n` >= 2^`num_bits`
    Args:
        num_bits (int) : number of bits in the generated integer

    Exceptions:
        ValueError : Number of bits is less than 2
    """
    while not is_prime((p := generate_prime_candidate(num_bits)), 128):
        pass

    return p


if __name__ == "__main__":
    num_args = len(sys.argv)
    if 2 <= num_args <= 3:
        num_bits = int(sys.argv[1])
        prime = generate_prime(num_bits)

        if num_args == 3 and sys.argv[2] == "--hex":
            s = hex(prime)
        else:
            s = str(prime)
        print(s)
    else:
        sys.stderr.write("usage: ./miller-rabin.py <num-bits> [--hex]\n")
        sys.exit(1)

    sys.exit(0)
