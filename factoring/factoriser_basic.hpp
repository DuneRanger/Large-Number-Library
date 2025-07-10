#pragma once
#include <cmath>
#include <fstream>
#include <cstdint>
#include <vector>
#include "../int_limited.hpp"
#include "factoriser_math.hpp"

namespace factoriser_basic {
	using largeNumberLibrary::int_limited;

	
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

	namespace {
		// globals list of primes, that is shared amongst all functions
		// Instead of relying on primes.txt containing them
		std::vector<uint64_t> primes;
		constexpr uint64_t max_val = 1000000 + 4;
	}

	void prepare_primes() {
		find_small_primes(max_val, primes);
	}

	// Finds all factors less than or equal to 1_000_003
	std::vector<uint64_t> trial_division(uint64_t value, uint64_t upper_bound = 0) {
		std::vector<uint64_t> factors;
		if (!primes.size()) prepare_primes();
		if (!upper_bound) upper_bound = UINT64_MAX;
		uint64_t max = std::ceil(std::sqrt(value));
		for (uint64_t prime : primes) {
			if (prime > upper_bound) break;
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

	// Finds all factors less than or equal to 1_000_003, or the given upper_bound
	template<int bit_size>
	std::vector<uint64_t> trial_division(int_limited<bit_size> value, uint64_t upper_bound = 0) {
		std::vector<uint64_t> factors;
		if (!primes.size()) prepare_primes();
		if (!upper_bound) upper_bound = UINT64_MAX;
		int_limited<bit_size> max = value.isqrt()+1;
		for (uint64_t prime : primes) {
			if (prime > upper_bound) break;
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
	bool is_small_prime(uint64_t value, uint64_t upper_bound = 0) {
		if (!primes.size()) prepare_primes();
		if (!upper_bound) upper_bound = UINT64_MAX;
		uint64_t max = std::ceil(std::sqrt(value));
		if (max > upper_bound) max = upper_bound;
		for (uint64_t prime : primes) {
			if (prime > max) return true;
			if (prime == value) return true;

			if (value % prime == 0) return false;
		}
		return true;
	}

	// Simple trial division, should work up to 10^12
	template<int bit_size>
	bool is_small_prime(int_limited<bit_size> value, uint64_t upper_bound = 0) {
		if (!primes.size()) prepare_primes();
		if (!upper_bound) upper_bound = UINT64_MAX;
		int_limited<bit_size> max = value.isqrt()+1;
		if (upper_bound < max) max = upper_bound;
		for (uint64_t prime : primes) {
			if (prime > max) return true;
			if (prime == value) return true;

			if (value % prime == 0) return false;
		}
		return true;
	}

	// A probabilistic Miller-Rabin primality test
	template<int bit_size>
	bool Miller_Rabin_test(int_limited<bit_size> const& n, uint64_t iterations = 25) {
		int_limited<2*bit_size> n_big = n;
		int_limited<2*bit_size> n_sub = n_big-1;
		int_limited<2*bit_size> d = n_sub;
		uint64_t s = 0;
		while ((uint64_t(d)&1) == 0) {
			d >>= 1;
			s++;
		}
		int_limited<2*bit_size> base_a = factoriser_math::random_64();
		for (int i = 0; i < iterations; i++) {
			int_limited<2*bit_size> a = factoriser_math::pow_mod<2*bit_size>(base_a, d, n_big);
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

	template<int bit_size>
	bool is_prime(int_limited<bit_size> const& N) {
		if (N == 2) return true;
		if ((uint64_t(N)&1) == 0) return false;
		if (N.ilog2() < 40) return is_small_prime(uint64_t(N));
		return Miller_Rabin_test(N);
	}
}
