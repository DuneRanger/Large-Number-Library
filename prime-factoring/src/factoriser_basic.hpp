#pragma once
#include <cstdint>
#include <vector>
#include "../../int_limited.hpp"

namespace factoriser_basic {
	using largeNumberLibrary::int_limited;
	
	// Finds all factors less than or equal to 1_000_003
	std::vector<uint64_t> trial_division(uint64_t value);

	// Finds all factors less than or equal to 1_000_003
	template<int bits>
	std::vector<uint64_t> trial_division(int_limited<bits> value);
	
	// Simple trial division, should work up to 10^12
	bool is_small_prime(uint64_t value);

	// Simple trial division, should work up to 10^12
	template<int bits>
	bool is_small_prime(int_limited<bits> value);

	// Adds all primes smaller or equal to max_val into the argument `primes`
	// Uses the sieve of Eratosthenes
	void find_small_primes(uint64_t max_val, std::vector<uint64_t>& primes);
}
