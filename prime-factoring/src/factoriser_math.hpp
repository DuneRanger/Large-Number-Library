#pragma once
#include "../../int_limited.hpp"
#include <cstdint>

namespace factoriser_math {
	using largeNumberLibrary::int_limited;

	// Calculates (n^exp) mod p without losing precision
	// Throws an error when p > UINT32_MAX (potential precision loss)
	uint64_t pow_mod(uint64_t n, uint64_t exp, uint32_t p);

	// Returns the Jacobi symbol for x (mod p)
	int calc_Jacobi_symbol(uint64_t x, uint64_t p);

	// Returns whether N is a quadratic residue modulo p
	// Returns false for N=0 (mod p)
	bool is_quadratic_residue(uint64_t N, uint64_t p);

	// Returns whether N is a quadratic residue modulo p
	// Returns false for N=0 (mod p)
	template<int bits>
	bool is_quadratic_residue(int_limited<bits> const& N, uint64_t p);

	// Returns the number of bits used in val
	// i.e. (64 - number of leading zeroes)
	uint64_t count_bits(uint64_t val);

	// Returns a both solutions to x^2 = N (mod p)
	// If a solution is not found, zero is returned
	// p must be a prime for the algorithm to work
	std::pair<uint64_t, uint64_t> Tonelli_Shanks(uint64_t N, uint64_t prime);

	// Returns a both solutions to x^2 = N (mod p)
	// If a solution is not found, zero is returned
	// p must be a prime for the algorithm to work
	template<int bits>
	std::pair<uint64_t, uint64_t> Tonelli_Shanks(int_limited<bits>& N, uint64_t prime);

	template<int bits>
	int_limited<bits> gcd(int_limited<bits> a, int_limited<bits> b);
}

