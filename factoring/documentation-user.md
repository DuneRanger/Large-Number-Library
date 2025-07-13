# Factorisation Library

## Table of Contents

- [Factorisation Library](#factorisation-library)
	- [Table of Contents](#table-of-contents)
	- [Usage](#usage)
	- [Factoriser](#factoriser)
		- [Debug options](#debug-options)
		- [Factoriser::Math](#factorisermath)
		- [Factoriser::Basic](#factoriserbasic)
		- [Factoriser::QuadraticSieve](#factoriserquadraticsieve)
	- [Examples](#examples)
		- [Example factoriser](#example-factoriser)
		- [Example QS](#example-qs)
		- [RSA testing](#rsa-testing)
		- [Factoriser CLI](#factoriser-cli)


## Usage

You can gain full usage of the library by including `./factoring/factoriser.hpp`, where the main functions are under the namespace `Factoriser::` and helper functions from other files are under their respective namespaces/classes (`Factoriser::Basic`, `Factoriser::Math`, `Factoriser::QuadraticSieve`).

Note: If the type `int_limited` is given without a template bit size, then any bit size can be used in that place.

## Factoriser

```cpp
std::vector<int_limited> factorise(int_limited value)
```
Returns all found prime factors of `value`

It processes `value` in three stages.
The first is simple trial division up to 1000 and then a primality test.
If the primality test fails, then trial division is attempted up to 100000, followed by another primality test.
If the primality test fails again (aka the input value had two prime factors larger than 100000), then an instance of the quadratic sieve algorithm is used until all prime factors have been found.

The output factors are returned in ascending order, being quickly sorted by `sort_factors()`, a bubble sort implementation suitable for sequences containing ascending subsequences.

**!Warning!** Due to the math behind the quadratic sieve, it is unable to factorise numbers which are powers of a *single* prime number (e.g. $1000003^2$).
In this scenario, the current behaviour of the `factorise()` function is to simply infinitely loop the quadratic sieve algorithm.
In the future, a n-th root factorising function will be implemented to cover these edge cases.

### Debug options

Three boolean debug options are given within the namespace.
Each of them turn on some level of debug logs for the `factorise()` function, which are all written to the standard output.

- `debug`: Turns on the basic debug logs for the `factorise()` function.
- `QS_debug`: Turns on basic quadratic sieve debug logs for the internal `QuadraticSieve` instance.
- `sieve_debug`: Turns on sieving logs for the internal `QuadraticSieve` instance (Warning, these can take up to hundreds of lines).

### Factoriser::Math

This is a namespace that defines some mathematical and helper functions used in the library.

---

```cpp
uint64_t pow_mod(uint64_t value,uint64_t exponent,uint64_t modulo)
int_limited pow_mod(int_limited value,int_limited const& exponent,int_limited const& modulo)
```

Returns `(value^exponent) % modulo`.
If there exists a possibility of overflowing and losing precision, then an error is thrown.

Higher precision is available by calling `pow_mod<2*bit_size>()`, which will convert the input values to `int_limited<2*bit_size>` integers, returning the output in the same type.
Because `int_limited` now supports assignment from different sizes and the output is guaranteed to be smaller than the input `modulo`, you can safely save the output value back into the original type of the input values.

---

```cpp
int calc_Jacobi_symbol(uint64_t value, uint64_t p)
```

Returns the Jacobi symbol for `value (mod p)`.

Because the Jacobi symbol is only defined for odd `p` (even non-primes), an error is thrown if `p` is even.
Based on the return value (0, 1, -1), either `value` is a multiple of `p`, `value` is a quadratic residue mod `p`, or `value` is a quadratic non-residue mod `p`.
If `p` is an odd prime number, then the Jacobi symbol is equal to the Legendre symbol.

---

```cpp
bool is_quadratic_residue(uint64_t value, uint64_t p)
bool is_quadratic_residue(int_limited const& value, uint64_t p)
```

Return `true` if `value` is a quadratic residue mod `p`.

This function is a simple extension of `calc_Jacobi_symbol()` that is capable of handling `p = 2` and thus all prime `p`.

---

```cpp
uint64_t count_bits(uint64_t value)
```

Simply returns the number of bits required to store `value` (i.e. 64 - `#Leading zeroes`).

---

```cpp
uint64_t Tonelli_Shanks(uint64_t N, uint64_t prime)
uint64_t Tonelli_Shanks(int_limited const& N, uint64_t prime)
```

Returns a single solution to $x^2 = N \mod p$ (the other solutions is $p - x$).
If a solution doesn't exist, zero is returned.

---

```cpp
int_limited gcd(int_limited const& a, int_limited const& b)
```

Returns the greatest common divisor of `a` and `b`

---

```cpp
uint64_t random_64(uint64_t n = 0)
```

Returns a pseudo-random 64 bit value based on either a non-zero input seed `n`, or an internal seed.

The internal seed always starts with the same value, so if different values are required for different runtimes, an input seed is required.

---

```cpp
template<typename T>
bool element_int_vector(T const& element, std::vector<T> const& list)
```

Returns whether `element` is present in `list` or not.
The implementation is a simple linear search.

### Factoriser::Basic

This is a namespace that defines basic functions for factorisation, like trial division, primality testing and the sieve of Eratosthenes.

Most functions use an shared vector of primes that is either initialised on their first call or with `prepare_primes()`.

---

```cpp
void find_small_primes(uint64_t max_value, std::vector<uint64_t>& primes)
```

Inserts all primes smaller than `max_value` into `primes`. Implements the sieve of Eratosthenes.

---

```cpp
void prepare_primes(uint64_t max_value = 1000000)
```

A helper function that prepares the vector of primes used in trial division and primality testing.

Because it calls `find_small_primes()`, it first clears the vector of primes.
If you expect to use a larger upper bound on other functions, consider calling `prepare_primes()` with the highest expected upper bound, so as to not experience unexpected slow downs.

---

```cpp
std::vector<uint64_t> trial_division(uint64_t value, uint64_t upper_bound = 1000000)
std::vector<uint64_t> trial_division(int_limited value, uint64_t upper_bound = 1000000)
```

Returns all prime factors of `value` that are less than `upper_bound`.

If upper_bound is larger than the largest prime in the shared vector, then a new shared vector of primes is generated to guarantee that all prime numbers lesser than `upper_bound` are tried.

`value` will be added to the vector of factors if it is found to be prime.

---

```cpp
bool is_small_prime(uint64_t value, uint64_t upper_bound = 1000000)
bool is_small_prime(int_limited value, uint64_t upper_bound = 1000000)
```

Returns `false` if `value` is found to be a composite number or is unable to be determined with the given `upper_bound`.

This means that for the default `upper_bound`, it will return the correct result for numbers less than $10^{12}$ and for larger numbers it will simply return `false`.

---

```cpp
bool Miller_Rabin_test(int_limited const& N, uint64_t iterations = 25)
```

Returns `true` if `N` is a strong probably prime for all iterations.

This function implements a probabilistic Miller-Rabin primality test.
With the default 25 iterations, it has at most a probability of $~8.89\cdot 10^{-16}$ to return `true` for a composite number.
If the input `N` is chosen randomly, then the probability decreases even further.

---

```cpp
bool is_prime(int_limited const& N)
```

Returns `true` if `N` is either a small prime (up to $10^{12}$) or if `N` is found to be a strong probable prime.

Because it tests primality deterministically up to $10^{12}$, the function may run faster for values after that threshold.

### Factoriser::QuadraticSieve

In comparison to others, this is a **class** definition.
It was chosen to be a class so as to hide the algorithm functions as private.
Because it is a class, it requires the `bit_size` of the expected arguments to be given ahead of time as a template argument. Like so:

```cpp
constexpr int bit_size = 256;
Factoriser::QuadraticSieve<bit_size> QS;
```

It has two public variables, booleans `debug` and `sieve_debug`, which can be also set during construction.
- `debug`: Turns on basic quadratic sieve debug logs documenting the variables chosen during factorisation.
- `sieve_debug`: Turns on sieving logs for the sieving phase of the algorithm (Warning, these can take up to hundreds of lines).

```cpp
std::vector<int_limited> factorise(uint64_t value)
std::vector<int_limited> factorise(int_limited const& value)
```

Returns a vector of found prime factors for the given value.

The output is not guaranteed to be a full prime factorisation of the input `value`, it is only likely to be so.
The algorithm specialises for numbers with a few large prime factors, though it should be capable of finding the factorisation of a number made up of a lot of small primes.

Because the algorithm work better for large numbers, values under 80 bits are artificially increased to help keep a consistent speed for small inputs (though that does not always guarantee a fast speed).

Notably, the algorithm is unable to factorise values with a factorisation of a single prime power - $prime^x$.
This is explained more in the [Developer Documentation](documentation-dev.md)

## Examples

Examples (and tests) of the main factorising functions (`Factoriser::factoriser()` and `Factoriser::QuadraticSieve`) can be found in the `./examples` directory.

### Example factoriser

This examples showcases the abilities of `Factoriser::factorise()` for a few inputs, measuring the time required for the factorisation to complete.

The starting value of the inputs can be changed in the source file, where a few other starting values can also be found.
Additionally, the amount of debug logs for each factorisation can be increased/decreased by changing `debug`, `QS_debug` and `sieve_debug` in the namespace. 

It is recommended stop testing values around 180 bits, due to long wait times (more than 10 minutes).

### Example QS

This example focuses on the quadratic sieve class and its general factorisation abilities, also measuring its factorisation speed.
The starting input value and the range of values can be changed in the source file.
Additionally, the amount of debug logs for each factorisation can be increased/decreased by changing `QS_debug` and `sieve_debug`. 

It is recommended stop testing values around 180 bits, due to long wait times (more than 10 minutes).

### RSA testing

This examples showcases the speed of the quadratic sieve using the standard method of factorising RSA encryption values (numbers made up of two large primes).

It uses four example numbers for 60, 70, 80, 100, 120, 140, 160 and 180 bit values, measuring the speed of each one individually and the average speed.

The very first 60 bit factorisation is noticeably slower than the subsequent ones.
This is most likely due to caching and other run-time dependant performance hits.

The amount of debug logs for each factorisation can be increased/decreased by changing `QS_debug` and `sieve_debug`. 

Here are some average results from a single run:

| Bits | 60 | 70 | 80 | 100 | 120 | 140 | 160 | 180 |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| Average time | 0.2247s | 0.1245s | 0.1851s | 0.3235s | 2.608s | 12.75s | 87.19s | 483.5s |

It is worth noting, that the most significant reason why these values may be found to be significantly slower than other implementations is due to this implementation utilising my own large number library, which surely does not have the most optimal implementation of large numbers.

### Factoriser CLI

This example is a factoriser instance that can be run from the command line.
The only accepted options are `-h` or `--help` and `-v` or `--verbose`.
There are four levels of verbosity: 0 (none), 1 (add factoriser logs), 2 (add quadratic sieve logs), 3 (add sieving phase logs).

Example usage:
```cpp
.\factoriser.exe 1298213469123407801234123432452345341

Expected output:
Factors: 3 3 3 3 3 1246459 4286095459030028499248303293
```
```cpp
.\factoriser.exe -v 1 1298213469123407801234123432452345341


Expected output:
=============== Factoriser input: 1298213469123407801234123432452345341 (120 bits) ===============
Stage 1: trial division up to 1000 & primality test
Stage 2: trial division up to 100000 & primality test
Stage 3: Quadratic sieve until the value is a strong probable prime to 25 bases
Factors: 3 3 3 3 3 1246459 4286095459030028499248303293 
Factorisation took 1.0549 seconds
```