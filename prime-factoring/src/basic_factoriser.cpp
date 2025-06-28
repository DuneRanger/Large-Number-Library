#pragma once
#include "headers.hpp"

// Finds all factors less than or equal to 999983
std::vector<uint64_t> trial_division(qs_int value) {
	std::vector<uint64_t> factors;
	std::ifstream primes("./src/primes.txt");
	int prime;
	qs_int max = value.isqrt()+1;
	while (primes >> prime) {
		if (max < prime) {
			std::cout << max << " " << prime << " " << value << std::endl;
			factors.push_back(uint64_t(value));
			break;
		}
		while (!(value % prime)) {
			factors.push_back(prime);
			value /= prime;
		}
		if (value == 1) break;
	}
	return factors;
}

// Simple trial division, should work up to 999983^2
bool is_small_prime(uint64_t value) {
	std::ifstream primes("./primes.txt");
	int prime;
	uint64_t max = std::ceil(std::sqrt(value));
	while (primes >> prime) {
		if (prime > max) break;
		while (!(value % prime)) value /= prime;
		if (value == 1) break;
	}
	return value == 1;
}

// Returns a vector of all primes smaller or equal to max_val
template<typename T>
std::vector<T> find_small_primes(T max_val) {
	std::vector<bool> sieve(max_val+1, true);

	for (T i = 2; i < max_val; i++) {
		if (sieve[i]) {
			for (T j = 2*i; j < max_val; j += i) sieve[j] = false;
		}
	}
	std::vector<T> primes;
	for (T i = 2; i < max_val; i++) {
		if (sieve[i]) primes.push_back(i);
	}
	return primes;
}