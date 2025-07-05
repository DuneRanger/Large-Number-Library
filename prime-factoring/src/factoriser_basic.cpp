#include <cmath>
#include <fstream>
#include "factoriser_basic.hpp"

namespace factoriser_basic {
	using largeNumberLibrary::int_limited;

	std::vector<uint64_t> trial_division(uint64_t value) {
		std::vector<uint64_t> factors;
		std::ifstream primes("./src/primes.txt");
		uint64_t prime;
		uint64_t max = std::ceil(std::sqrt(value));
		while (primes >> prime) {
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

	template<int bits>
	std::vector<uint64_t> trial_division(int_limited<bits> value) {
		std::vector<uint64_t> factors;
		std::ifstream primes("./src/primes.txt");
		int prime;
		int_limited<bits> max = value.isqrt()+1;
		while (primes >> prime) {
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

	bool is_small_prime(uint64_t value) {
		std::ifstream primes("./primes.txt");
		uint64_t prime;
		uint64_t max = std::ceil(std::sqrt(value));
		while (primes >> prime) {
			if (prime > max) return true;
			while (!(value % prime)) value /= prime;
			if (value == 1) return true;
			max = std::ceil(std::sqrt(value));
		}
		return false;
	}

	template<int bits>
	bool is_small_prime(int_limited<bits> value) {
		std::ifstream primes("./primes.txt");
		uint64_t prime;
		int_limited<bits> max = value.isqrt()+1;
		while (primes >> prime) {
			if (prime > max) return true;
			while (!(value % prime)) value /= prime;
			if (value == 1) return true;
			max = value.isqrt()+1;
		}
		return false;
	}

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
