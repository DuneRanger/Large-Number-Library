#pragma once
#include <cmath>
#include <fstream>
#include <cstdint>
#include <vector>
#include "factoriser_basic.hpp"
#include "../../int_limited.hpp"

namespace factoriser_basic {
	using largeNumberLibrary::int_limited;

	// Finds all factors less than or equal to 1_000_003
	std::vector<uint64_t> trial_division(uint64_t value, uint64_t upperBound = 0) {
		std::vector<uint64_t> factors;
		std::ifstream primes("./src/primes.txt");
		uint64_t prime;
		if (!upperBound) upperBound = UINT64_MAX;
		uint64_t max = std::ceil(std::sqrt(value));
		while (primes >> prime && prime < upperBound) {
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
		while (primes >> prime && prime < upperBound) {
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
		std::ifstream primes("./primes.txt");
		uint64_t prime;
		if (!upperBound) upperBound = UINT64_MAX;
		uint64_t max = std::ceil(std::sqrt(value));
		while (primes >> prime && prime < upperBound) {
			if (prime > max) return true;
			while (!(value % prime)) value /= prime;
			if (value == 1) return true;
			max = std::ceil(std::sqrt(value));
		}
		return false;
	}

	// Simple trial division, should work up to 10^12
	template<int bits>
	bool is_small_prime(int_limited<bits> value, uint64_t upperBound = 0) {
		std::ifstream primes("./primes.txt");
		uint64_t prime;
		if (!upperBound) upperBound = UINT64_MAX;
		int_limited<bits> max = value.isqrt()+1;
		while (primes >> prime && prime < upperBound) {
			if (prime > max) return true;
			while (!(value % prime)) value /= prime;
			if (value == 1) return true;
			max = value.isqrt()+1;
		}
		return false;
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
}
