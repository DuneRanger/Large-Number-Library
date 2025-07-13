# Factorisation Library

## Table of Contents

- [Factorisation Library](#factorisation-library)
	- [Table of Contents](#table-of-contents)
	- [General information](#general-information)
	- [Factoriser](#factoriser)
		- [Factoriser::Math](#factorisermath)
		- [Factoriser::Basic](#factoriserbasic)
		- [Factoriser::QuadraticSieve](#factoriserquadraticsieve)
			- [Public](#public)
			- [Private](#private)
				- [Structs \& Classes](#structs--classes)
					- [Custom Bitset](#custom-bitset)
					- [QS poly](#qs-poly)
					- [Relation](#relation)
					- [QS global](#qs-global)
				- [Preparation](#preparation)
				- [Sieving](#sieving)
					- [Finding Relation Candidates](#finding-relation-candidates)
					- [Verifying Relation Candidates](#verifying-relation-candidates)
					- [End of Sieving](#end-of-sieving)
				- [Finding Factors from Relations](#finding-factors-from-relations)
					- [Linear Algebra](#linear-algebra)
					- [Finding Divisors](#finding-divisors)
			- [Miscellaneous Additions](#miscellaneous-additions)
	- [Sources Used During Implementation](#sources-used-during-implementation)

## General information

This documentation serves to explain some design choices behind some function implementations and to explain what was considered at the time.

## Factoriser

This is the main namespace of the library and should only contains variables and functions pertaining to the main `factorise()` function.

Currently that includes three debug options and a function to sort factors.
The debug options do not influence any debug options in other files.

---

```cpp
std::vector<int_limited> factorise(int_limited value)
```

It processes `value` in three stages.
The first is simple trial division up to 1000 and then a primality test.
If the primality test fails, then trial division is attempted up to 100000 (1e5), followed by another primality test.
If the primality test fails again (aka the input value had two prime factors larger than 100000), then an instance of the quadratic sieve algorithm is used until all prime factors have been found.

Testing trial division up to 1000000 (1e6) was tested and found to be rather inefficient, drastically slowing down the factorisation of smaller values that the quadratic sieve could handle faster.

The output factors are returned in ascending order, being quickly sorted by `sort_factors()`, a bubble sort implementation suitable for sequences containing ascending subsequences.

This is required because the quadratic sieve class does not sort the found factors before returning them

**!Warning!** Due to the math behind the quadratic sieve, it is unable to factorise numbers which are powers of a *single* prime number (e.g. $1000003^2$).
In this scenario, the current behaviour of the `factorise()` function is to simply infinitely loop the quadratic sieve algorithm.
In the future, a n-th root factorising function will be implemented to cover these edge cases.

### Factoriser::Math

This is a namespace that defines some mathematical and helper functions used in the library.

---

```cpp
uint64_t pow_mod(uint64_t value,uint64_t exponent,uint64_t modulo)
int_limited pow_mod(int_limited value,int_limited const& exponent,int_limited const& modulo)
```

Returns `(value^exponent) % modulo`.
If there exists a possibility of overflowing and losing precision, then an error is thrown.

Currently, the error is thrown with the intermediate value of `value` being given.
It may be interesting to explore saving the original value for the error, however this isn't exactly a priority, so maybe just removing the actual value from the error is a valid solution.

---

```cpp
int calc_Jacobi_symbol(uint64_t value, uint64_t p)
```

Returns the Jacobi symbol for `value (mod p)`.

Because the Jacobi symbol is only defined for odd `p` (even non-primes), an error is thrown if `p` is even.
If `p` is an odd prime number, then the Jacobi symbol is equal to the Legendre symbol.

The Jacobi symbol was chosen over the Legendre symbol simply because I didn't have a need for the `pow_mod()` function yet and didn't want a single function to be the reason to implement it.

As for the Kronecker symbol, the more general definition of the Jacobi symbol, I simply didn't know it existed at the time of implementation, thus I stuck with the Jacobi symbol.

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
Can be used as `floor(log2(value))` for any fast log approximations.

---

```cpp
uint64_t Tonelli_Shanks(uint64_t N, uint64_t prime)
uint64_t Tonelli_Shanks(int_limited const& N, uint64_t prime)
```

Returns a single solution to $x^2 = N \mod p$ (the other solutions is $p - x$).
If a solution doesn't exist, zero is returned.

The function only returns a single solution so that it is simpler the recognize that not solution exists.
The implementation is mostly from the lua pseudo code from [Wikipedia](https://en.wikipedia.org/wiki/Tonelli%E2%80%93Shanks_algorithm#The_algorithm)

The function doesn't consider the possibility of integer overflow.
It may be worth exploring an overload for an `int_limited prime` that can be called for higher bit values

---

```cpp
int_limited gcd(int_limited const& a, int_limited const& b)
```

Returns the greatest common divisor of `a` and `b`.
Nothing special here.

---

```cpp
uint64_t random_64(uint64_t n = 0)
```

Returns a pseudo-random 64 bit value based on either a non-zero input seed `n`, or an internal seed.

This is a fun one.
I originally wanted to reuse the 64 bit number generator from `int_limited` testing (from [Reddit](https://www.reddit.com/r/C_Programming/comments/ozew2u/comment/h7zijm8)), however there was a need for the Miller-Rabin test to have random integers throughout runtime, not just every time it is called.

Supposedly the option of having a static seed for Miller-Rabin exists, but instead I just added it directly to this function, together with some untested bit randomiser to keep the seed at least somewhat unrecognisable between calls.

---

```cpp
template<typename T>
bool element_int_vector(T const& element, std::vector<T> const& list)
```

Returns whether `element` is present in `list` or not.
The implementation is a simple linear search.

This exists mainly due to its usage in the `QaudraticSieve` class, however I decided to define it here in case it might have some use for the main `factoriser()` function in the future.

### Factoriser::Basic

This is a namespace that defines basic functions for factorisation, like trial division, primality testing and the sieve of Eratosthenes.

Most of the function implementations here require some sort of external source of prime numbers to iterate through.
Originally this was a text file `primes.txt`, however due to the annoyance of having to have to find its path for each of these functions, a shared vector in an anonymous namespace became the new solutions.
Surprisingly enough, this change alone seemed to slow done quadratic sieve run times by 1.5x.

Because the variable lies within a namespace, there is no clear cut time when to properly initialise it, thus the `prepare_primes()` function was born.
More information on this shared vectors behaviour will be described there.

---

```cpp
void find_small_primes(uint64_t max_value, std::vector<uint64_t>& primes)
```

Inserts all primes smaller than `max_value` into `primes`. Implements the sieve of Eratosthenes.

It takes the output vector as an argument to save time copying over all of the values.
The implementation uses a vector of bools, so it should be relatively memory efficient, however it may slow down the run time a bit.

In the end, that should be negligible though, since the function isn't intended to be on you use often.

---

```cpp
void prepare_primes(uint64_t max_value = 1000000)
```

A helper function that prepares the vector of primes used in trial division and primality testing.

Because it calls `find_small_primes()`, it first clears the vector of primes, before filling again.
Originally the max value was supposed to be a constant, however it became a parameter to support upper bounds for other functions.

Specifically, other functions utilising the vector of primes are expected to always run up until their upper bound. If the last prime in the shared vector is lesser than the upper bound, then `prepare_primes(2*upper_bound)` is called to ensure that an additional prime number larger than `upper_bound` is added.

Realistically, this shouldn't be too much of a slowdown, and due to its newly dynamic nature, perhaps its time to reduce the default max value.

---

```cpp
std::vector<uint64_t> trial_division(uint64_t value, uint64_t upper_bound = 1000000)
std::vector<uint64_t> trial_division(int_limited value, uint64_t upper_bound = 1000000)
```

Returns all prime factors of `value` that are less than `upper_bound`.

If upper_bound is larger than the largest prime in the shared vector, then a new shared vector of primes is generated to guarantee that all prime numbers lesser than `upper_bound` are tried (see `prepare_primes()`).

The maximum prime value (the square root of `value`) is recalculated every time a factor is found, so as to keep it as low as possible.

`value` will be added to the vector of factors if it is found to be prime (i.e. we reach a prime larger than the maximum prime value).

---

```cpp
bool is_small_prime(uint64_t value, uint64_t upper_bound = 1000000)
bool is_small_prime(int_limited value, uint64_t upper_bound = 1000000)
```

Returns `false` if `value` is found to be a composite number or is unable to be determined with the given `upper_bound`.

This means that for the default `upper_bound`, it will return the correct result for numbers less than $10^{12}$ and for larger numbers it will simply return `false`.

Its internal behaviour is similar to `trial_division()`, however it can return as soon as one factor is found.

---

```cpp
bool Miller_Rabin_test(int_limited const& N, uint64_t iterations = 25)
```

Returns `true` if `N` is a strong probably prime for all iterations.

This function implements a probabilistic Miller-Rabin primality test.
With the default 25 iterations, it has at most a probability of $~8.89\cdot 10^{-16}$ to return `true` for a composite number.
If the input `N` is chosen randomly, then the probability decreases even further.

Internally, double bit size `int_limited` are used to guarantee that an overflow does not occur when `pow_mod`ing with the randomly generated base `base_a`.

`base_a` is generated by shifting itself left by 32 bits and then xoring 64 random bits from `random_64` (see the end of [Factoriser::Math](#factorisermath)), so that a few "small" values of `base_a` are attempted before generating large values.

---

```cpp
bool is_prime(int_limited const& N)
```

Returns `true` if `N` is either a small prime (up to $10^{12}$) or if `N` is found to be a strong probable prime.

The small prime testing occurs below the threshold of 40 bits, using `is_small_prime()` with an upper bound of 1000000 ($1e6$).
Actually testing a value near $10^{12}$ can start being a bit slow, so it may be worth considering lowering the threshold to 1000000^2 (1e5^2), or ideally implementing the slower deterministic variant of the Miller-Rabin primality test, thus completely removing the requirement for a large list of primes when testing primality.

### Factoriser::QuadraticSieve

In comparison to others, this is a **class** definition.
It was chosen to be a class so as to hide the algorithm functions as private.
Anonymous namespaces were also considered, however at the time of implementation, I wasn't sure whether they truly hid everything, so for simplicities sake I stuck with a class.

This class implements a relatively optimized single polynomial version of the [quadratic sieve](https://en.wikipedia.org/wiki/Quadratic_sieve) algorithm.

Internally, `qs_int` and `qs_int_double` are used to represent the input `int_limited` type and a version that is double its bit size.

#### Public

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
The math behind this is simple when $x = 2$ is considered.

Let $N = p^2$. The algorithm searches for an $A$ and $B$ such that $(A + B)(A - B) = 0 \mod N$, however if $A + B = A - B$, then $B$ must equal zero, in which case searching for $A$ requires $p$ to actually be contained within the factor base, which is infeasible for any large primes.

For this reason a separate n-th root function must be implemented to help factorise values made up of a single prime power.

#### Private

##### Structs & Classes

###### Custom Bitset

This class is used to represent rows of a matrix modulo 2.
Because I couldn't be bothered to make the indexing operator `[]` return a proxy, it only allows read access to a single bit and overwriting a value is done with the `flip_bit()` method.

The xor operator was implemented for easy matrix row addition.
The equality operator was implemented for testing whether removing duplicate rows of the matrix has a significant effect on the algorithm.
Surprisingly enough, barely any rows were found to be duplicates, though this testing didn't remove singleton columns, so maybe that would make more of a difference.

###### QS poly

This purpose of this struct is to store variables for a single polynomial used in the quadratic sieve sieving process.
The implementation should be generic enough for it to be usable if the quadratic sieve algorithm was to be upgraded to a multi-polynomial version.
Though if we reached a self-initializing version of the algorithm, I don't doubt that the struct would be overhauled anyway.

The current implementations saves the variables $A, B$ and $C$ for a polynomial in the form of $Ax^2 + Bx + C$, with the `()` operator returning the value for a given $x$.

There is also a special constructor used when for when the polynomial is supposed to be in the form of $(\sqrt{kN}  + x)^2 - kN$.

Finally, each polynomial remembers a vector of its solutions to the equation $Q(x) = 0 \mod p$, where $p$ is a prime of same index in the factor base.

###### Relation

This struct is used to store related information for each relation found during the sieving process.
Currently, that means:
- `qs_int poly_value`: The value of the polynomial for the relation, the right hand side of the relation.
- `qs_int residue_solution`: The solution to the equation $R^2 = 0 \mod kN$, the left hand side of the relation
- `vector<uint64_t> exponents`: The prime factorisation  of `poly_value` in the form of a vector containing the power of each prime in the factor base. \
This is used to drastically increase the speed of [finding factors from relations](#finding-factors-from-relations), by allowing us the square each power during multiplication, allowing us to work modulo $N$, instead of multiplying up to 10000+ bit values.
- `CustomBitset exponents_mod_2`: This simply stores the form of the relation for the matrix modulo 2. \
This may be removed in favour of recreating it from `exponents` when needed.

###### QS global

This struct mainly helps reduce the number of arguments used when passing values between functions, without resorting to global variables that would need to be cleared after use.

It currently stores:
- `qs_int N`: The input value.
- `qs_int kN`: The modified input value more suitable for gathering relations.
- `uint64_t B`: The smoothness bound for relations, aka the maximum value of a prime in the factor base.
- `vector<uint64_t> factor_base`: A vector of every prime number in the factor base.
- `int64_t sieve_start`: The start of the current sieving interval.
It is unsigned to account for negative values in the potential future (multi-polynomial QS)
- `uint64_t sieve_interval`: The length of the current sieving interval.

##### Preparation

This section accounts for everything up to the actual sieving process.

1) Calculating a suitable `kN` to work with.
As I currently do not understand all the intricacies behind this, all it means is setting `kN` to be at least 80 bits to enlarge the factor base.
2) Calculating a smoothness bound `B`. Based on this [article](https://medium.com/nerd-for-tech/heres-how-quadratic-sieve-factorization-works-1c878bc94f81), the complexity of the algorithm is a suitable estimate (since the complexity is based on the chances to find relations). \
From personal testing, the difference between simply using `log2` and `ln` is very much noticeable and even a approximation of $ln(2) \approx \frac{2}{3}$ is enough to significantly reduce `B` whilst speeding up the algorithm
3) Preparing the factor base.
This simply consists of taking every prime less than `B` for which `kN` is a quadratic residue ($\exists x \in \Z_p: x^2 = kN \mod p$)
4) Preparing the polynomials.
This is also simple for the single-polynomial version.
The only extra detail is that the solution to $Q(x) = 0 \mod p$ is calculated for every prime in the factor base, so that we don't need to calculate it anew during the sieving process.
5) Setting the initial sieving bounds.
It's rather hard to choose a suitable sieving interval.
In my opinion, it must be tied in some form to `B` and having a large interval could save a bit of time for initialising values.
However keeping the interval small helps stop the sieving process as soon as enough relations are found.
A bit more to the subject will be explained in the [sieving section](#sieving).

##### Sieving

A single sieve instance can be started with the `sieve()` function.

The goal of sieving is to find inputs $x$ for which the value of a given polynomial has a prime factorisation fully contained within the factor base.
Finding out if a value is divisible by a prime in the factor base is done with the help of solving the polynomial modulo p once, saving it in `solutions_mod_p` and then every value of $Q(x + p)$ is divisible by $p$ when $Q(x)$ is divisible by $p$.

A naive solution to figure out which values are `B` smooth is to calculate $Q(x)$ for every $x$ and then divide every value $Q(x + p)$ by $p$ for every prime in the factor base. And at the end, any value left at $1$ is `B` smooth.

This is however incredibly slow, even just the requirement of calculating $Q(x)$ for every $x$, thus an approximation is used, which divides this section into two phases.

###### Finding Relation Candidates

First of all, instead of dividing every $Q(x + p)$ by $p$, we instead add $\log(p)$ to an approximation of $\log(Q(x + p))$.
Then we take all values of $x$ for which the approximation of $log(Q(x))$ is above a certain threshold.
We will call these values *candidates*

The ideal threshold would simply be $\log(Q(x)) \cdot k$, where $k \in (0;1)$, for all $x$, however, as was mentioned before, calculating every $Q(x)$ is slow, thus we calculate the threshold for a single $Q(x)$ and use it for all values.

This means we only need to calculate one polynomial and precalculate every $\log p$ for the approximation.
The approximation does favour candidates containing larger primes and low exponents, however in practice it can find valid relations much faster, even with a very strict threshold.

Specifically, the threshold chosen for the implementation was $\log(Q(start + \frac{interval}{10})) \cdot \frac{7}{8}$.
Although this doesn't find a lot of valid relations, it still finds a significant amount compared to $k = \frac{3}{4}$, whilst also rarely finding bad candidates for relations.

The reason why the threshold was chosen to be so strict is explained in the next phase, the candidate verification process.

###### Verifying Relation Candidates

As far as I know, the only proper way to verify relation candidates is the factorise them with trial division.
With factor bases reaching 10000+ primes, this can be a bit of a slow process for a large amount of candidates.

In fact, because I use my own large integer implementation, instead of an optimized library, this is the slowest part of the sieving process.
For this reason, attempting to verify invalid candidates is a considerable slowdown that doesn't contribute to the algorithm.
Thus, the threshold for relation candidates was set to be strict enough, that usually less that 1% of candidates are invalid relations.

Of course, since we are already conducting trial division of each relation here, we can also keep track of their prime exponents, so that we can later use them in the linear algebra phase and when finding factors.

###### End of Sieving

At the end of the sieving process, the sieving start value is updated to where the sieving ended.
I also decided to add a condition to increase the sieving interval if not even one tenth of the required relations were found.

This was done mainly because with the strict threshold and slow verification, we spend most of the time verifying valid relations anyway, so there might be some time to be saved by simply sieving over a larger interval at once, so as to not have so many intervals without any valid relations found.

##### Finding Factors from Relations

###### Linear Algebra

As far as I know, there are two common implementations of this phase of the algorithm.
One is the simple gauss elimination and the other is the Block Lanczos algorithm, suitable for solving sparse matrices.

According to [this masters thesis](https://dspace.cvut.cz/bitstream/handle/10467/94585/F8-DP-2021-Vladyka-Ondrej-DP_Vladyka_Ondrej_2021.pdf) and from personal experimentation, the time spent in the linear algebra phase is only a fraction of the algorithms runtime (I would personally estimate it to be at most $\frac{1}{4}$, usually around $\frac{1}{8}$).
So I decided to keep it simple by implementing gauss elimination.

Because we are actually solving the left null space, we find the solution by mirroring every step of the gauss elimination in an identity matrix representing every relation.
Then we simply return all rows of the solution matrix for which we were able to completely zero out the original relation in the matrix.

###### Finding Divisors

First of all, every solution of the matrix has at least a 1 in 2 chance of being a non-trivial factor.
However, going through every solution isn't ideal when we might end up finding over a thousand.
So I decided to cap the amount of solutions processed to 50, leaving only a small chance that the input value isn't completely factorised.

Finding a divisor requires multiplying every `poly_value` and every `residue_solution` from every relation in the given matrix solution.
Then we end up with $A^2 = B^2 \mod N$ from which we require $A$ and $B$.

Because we can guarantee that every `residue_solution` is in the form of $(A + x)^2$, we only need to save and multiply the square root of the value, which allows us to also work modulo $N$, keeping the intermediate values small.

The saved `poly_value`s however are not so fortunate, and because we need to square root the final value, we cannot simply work modulo $N$ without any other changes.

This is the reason every relation also remembers its exponents for every prime of the factor base, so that we can add up the exponents, divide them all by two to square root the total value and only then multiply all of them together modulo $N$.

Without using this method, the intermediate values can reach over 10000 bits long, resulting in very poor performance for unoptimized multiplication.
Interestingly enough, an [implementation for a masters thesis](https://dspace.cvut.cz/handle/10467/94585) using the GMP library does not handle this issue, whilst boasting much faster times.
So I imagine that a large part of my implementations performance is hindered by my `int_limited` implementation.

After all that, the divisors are found from $\gcd(A - B, N)$ and $\gcd(A + B, N)$.
We only save unique divisors for processing, meaning that they are then either confirmed to be (strong probable) primes, or they are factorised either by trial division or another quadratic sieve instance.

Then we test the divisibility of the input value `N` by all of the found primes and return all valid prime divisors.

#### Miscellaneous Additions

As a bit of a counter measure towards repeated failures to factorise an input value, saves the last input value that wasn't factored and where its sieving ended.
Then, if the input value matches the last value that wasn't factored, the sieving starts from where it last ended, instead of sieving the same interval from the default sieve start.

## Sources Used During Implementation

- [A medium.com article explaining the basic concept of the algorithm (Akintunde Ayodele)](https://medium.com/nerd-for-tech/heres-how-quadratic-sieve-factorization-works-1c878bc94f81)
- [The quadratic sieve algorithm wikipedia page](https://en.wikipedia.org/wiki/Quadratic_sieve)
- [The Tonelli-Shanks algorithm wikipedia page](https://en.wikipedia.org/wiki/Tonelli%E2%80%93Shanks_algorithm)
- [The Jacobi symbol wikipedia page](https://en.wikipedia.org/wiki/Jacobi_symbol)
- [The Miller-Rabin primality test wikipedia page](https://en.wikipedia.org/wiki/Miller%E2%80%93Rabin_primality_test)
- [An introduction to the theory behind the quadratic sieve and implementation issues (RNDr. Marian Kechlibar)](https://www.karlin.mff.cuni.cz/~krypto/Implementace_MPQS_SIQS_files/main_file.pdf)
  - The implementation issues were more pertaining the algorithms I didn't end up implementing, however this was a much needed source behind the theory of the algorithm.
- [An advanced quadratic sieve implementation in pure C (Michel Leonard)](https://github.com/michel-leonard/C-Quadratic-Sieve/tree/main)
  - Although I can't even come close to understanding all of the code, I was able to gleam a few helpful insights from here, such as the exponent saving of `poly_value` to use when finding factors from relations
- [A bare bones naive quadratic sieve implementation in python (cramppet)](https://github.com/cramppet/quadratic-sieve/tree/master)
  - I found this towards the end of my implementation, but was still nice to have as a comparison to see just how many optimisations were made in my implementation
- [A masters thesis containing the implementation of different versions of the quadratic sieve algorithm in C++ (Ondřej Vladyka)](https://dspace.cvut.cz/handle/10467/94585)
  - Reading through this implementation definitely gave me a better sense of whether I was on the right path or if I was doing something completely wrong. It also made me realise just how important setting a simple sieving threshold is and how it drastically increases sieving efficiency.