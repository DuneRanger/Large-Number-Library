#pragma once
#include <cmath>
#include <fstream>
#include <cstdint>
#include <vector>
#include "../../int_limited.hpp"
#include "factoriser_math.hpp"

namespace factoriser_basic {
	using largeNumberLibrary::int_limited;

	// Finds all factors less than or equal to 1_000_003
	std::vector<uint64_t> trial_division(uint64_t value, uint64_t upperBound = 0) {
		std::vector<uint64_t> factors;
		std::ifstream primes("./src/primes.txt");
		uint64_t prime;
		if (!upperBound) upperBound = UINT64_MAX;
		uint64_t max = std::ceil(std::sqrt(value));
		while ((primes >> prime) && prime < upperBound) {
			if (max < prime) {
				factors.push_back(uint64_t(value));
				break;
			}
			while (!(value % prime)) {
				factors.push_back(prime);
				value /= prime;
				max = std::ceil(std::sqrt(value));
			}
			if (value == 1) break;
		}
		return factors;
	}

	// Finds all factors less than or equal to 1_000_003
	template<int bits>
	std::vector<uint64_t> trial_division(int_limited<bits> value, uint64_t upperBound = 0) {
		std::vector<uint64_t> factors;
		std::ifstream primes("./src/primes.txt");
		uint64_t prime;
		if (!upperBound) upperBound = UINT64_MAX;
		int_limited<bits> max = value.isqrt()+1;
		while ((primes >> prime) && prime < upperBound) {
			if (max < prime) {
				factors.push_back(uint64_t(value));
				break;
			}
			while (!(value % prime)) {
				factors.push_back(prime);
				value /= prime;
			}
			if (value == 1) break;
			max = value.isqrt()+1;
		}
		return factors;
	}

	// Simple trial division, should work up to 10^12
	bool is_small_prime(uint64_t value, uint64_t upperBound = 0) {
		std::ifstream primes("./src/primes.txt");
		uint64_t prime = 0;
		if (!upperBound) upperBound = UINT64_MAX;
		uint64_t max = std::ceil(std::sqrt(value));
		while ((primes >> prime) && prime < upperBound) {
			if (prime > max) return true;
			if (prime == value) return true;

			if (value % prime == 0) return false;
		}
		return true;
	}

	// Simple trial division, should work up to 10^12
	template<int bits>
	bool is_small_prime(int_limited<bits> value, uint64_t upperBound = 0) {
		std::ifstream primes("./src/primes.txt");
		uint64_t prime = 0;
		if (!upperBound) upperBound = UINT64_MAX;
		int_limited<bits> max = value.isqrt()+1;
		while ((primes >> prime) && prime < upperBound) {
			if (prime > max) return true;
			if (prime == value) return true;

			if (value % prime == 0) return false;
		}
		return true;
	}

	// Adds all primes smaller or equal to max_val into the argument `primes`
	// Uses the sieve of Eratosthenes
	void find_small_primes(uint64_t max_val, std::vector<uint64_t>& primes) {
		std::vector<bool> sieve(max_val+1, true);

		for (uint64_t i = 2; i < max_val; i++) {
			if (sieve[i]) {
				primes.push_back(i);
				for (uint64_t j = 2*i; j < max_val; j += i) sieve[j] = false;
			}
		}
	}

	// A probabilistic Miller-Rabin primality test
	template<int bits>
	bool Miller_Rabin_test(int_limited<bits> const& n, uint64_t iterations = 25) {
		int_limited<2*bits> n_big = n;
		int_limited<2*bits> n_sub = n_big-1;
		int_limited<2*bits> d = n_sub;
		uint64_t s = 0;
		while ((uint64_t(d)&1) == 0) {
			d >>= 1;
			s++;
		}
		int_limited<2*bits> base_a = factoriser_math::random_64();
		for (int i = 0; i < iterations; i++) {
			int_limited<2*bits> a = factoriser_math::pow_mod<2*bits>(base_a, d, n_big);
			if (a == 1 || a == n_sub) continue; // is a strong probable prime to base a
			int j = 1;
			for (; j < s; j++) {
				a = a*a%n_big;
				if (a == n_sub) break;
			}
			if (j == s) return false; // isn't a strong probably prime, thus it is composite
			base_a <<= 32;
			if (base_a < 0) base_a >>= 1;
			base_a ^= factoriser_math::random_64();
			base_a %= n_big;
		}
		return true;
	}

	template<int bits>
	bool is_prime(int_limited<bits> const& N) {
		if (N == 2) return true;
		if ((uint64_t(N)&1) == 0) return false;
		if (N.ilog2() < 40) return is_small_prime(uint64_t(N));
		return Miller_Rabin_test(N);
	}
}
