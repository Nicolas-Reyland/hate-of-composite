# my\_prime - generation

*my_prime* is a tool written in C, whith it's primary focus being security. It can generate prime numbers using a CSPRNG that is part of the [Fortuna](https://en.wikipedia.org/wiki/Fortuna_%28PRNG%29) family of CSPRNG. It can also check the primality of numbers. Primality checks are done using the following logic :
 1. A series of trivial checks are performed (is the number 0, 1, 2 or negative, etc)
 2. The divisibility with small prime numbers is checked (the test very often stops here)
 3. A [Miller-Rabin primality test](https://en.wikipedia.org/wiki/Miller%E2%80%93Rabin_primality_test) is executed
 4. If the number has passed all these tests, then it is considered a prime number (with a very high probability)

The test changes slightly when generating primes vs testing the primality of a single number, but the logic remains the same. The CSPRNG is not the same when testing the primality of a single number, since initializing the CSPRNG is really costy, and it is more efficient to simply using cryptographically secure random bytes provided by the system itself than using the Fortuna CSPRNG for a single run of miller-rabin tests.
Although, using cryptographically secure random bytes provided by the system is slower on the long run (e.g. when generating prime numbers), since the operating system has to wait until enough entropy is present to provide the random bytes (see [this link](https://man7.org/linux/man-pages/man2/getrandom.2.html)).

