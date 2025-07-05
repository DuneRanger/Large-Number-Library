#pragma once
#include <cstdint>
#include "factoriser_math.hpp"

namespace factoriser_math {
	using largeNumberLibrary::int_limited;

	// Calculates (n^exp) mod p without losing precision
	// Throws an error when p > UINT32_MAX (potential precision loss)
	uint64_t pow_mod(uint64_t n, uint64_t exp, uint64_t p) {
		if (p > UINT32_MAX) throw std::overflow_error("Error: pow_mod with p = " + std::to_string(p) + " may overflow");
		if (exp == 0) return 1;
		if (exp % 2) return (pow_mod(n, exp-1, p) * n) % p;
		n = pow_mod(n, exp/2, p);
		return (n * n) % p;
	}

	// Returns the Jacobi symbol for x (mod p)
	int calc_Jacobi_symbol(uint64_t x, uint64_t p) {
		if (p%2 == 0) throw std::domain_error("Error: An even denominator was used when calculating Jacobi symbol");
		uint64_t a = x%p;
		int symbol = 1;
		// until we reach the base case (p will be 1 if they are coprime)
		while (a != 0) {
			// First change a to be an odd number
			// (2a | n) = (2 | n) * (a | n)
			while (a%2 == 0) {
				a /= 2;
				// (2 | n) = (-1)^((n^2 - 1) / 8)
				switch(p%8) {
					case 3: case 5:
					symbol = -symbol;
					break;
					// in the other cases, the symbol stays the same
				}
			}
			// this section only applies if a,p are coprime
			// but if they aren't, then we can guarantee p will be non-zero at the end
			// (a | p)*(p | a) = (-1)^((a-1)/2 * (p-1)/2)
			// so (a | p) = (-1)^((a-1)/2*(p-1)/2) / (p | a)
			// and (-1)^((a-1)/2*(p-1)/2) is -1 when the following applies
			if (a%4 == 3 && p%4 == 3) symbol = -symbol;
			uint64_t t = a;
			a = p % a;
			p = t;
		}

		if (p != 1) return 0;
		else return symbol;
	}

	// Returns whether N is a quadratic residue modulo p
	// Returns false for N=0 (mod p)
	bool is_quadratic_residue(uint64_t N, uint64_t p) {
		if (p == 2) return N&1;
		return calc_Jacobi_symbol(N, p) == 1;
	}

	// Returns whether N is a quadratic residue modulo p
	// Returns false for N=0 (mod p)
	template<int bits>
	bool is_quadratic_residue(int_limited<bits> const& N, uint64_t p) {
		if (p == 2) return uint64_t(N)&1;
		return calc_Jacobi_symbol(uint64_t(N%p), p) == 1;
	}
	
	// Returns the number of bits used in val
	// i.e. (64 - number of leading zeroes)
	uint64_t count_bits(uint64_t val) {
		uint64_t bits = 0;
		for (uint64_t i = 32; i > 0; i >>= 1) {
			uint64_t t = val >> i;
			if (t) { val = t; bits += i; }
		}
		if (val) bits += 1;
		return bits;
	}

	// Mostly just implements the pseudo code from:
	// https://en.wikipedia.org/wiki/Tonelli%E2%80%93Shanks_algorithm#The_algorithm
	uint64_t Tonelli_Shanks(uint64_t N, uint64_t prime) {
		if (prime == 2) return N&1;
		N %= prime;
		if (N == 0) return 0;
		
		// We're solving x^2 = N (mod p)
		// because n^((p-1)/2) = 1 (mod p), we find Q,S such that Q*2^S = p-1
		uint64_t Q = prime-1, S = 0;
		while (Q%2 == 0) { Q /= 2; S++; }

		// Next we find a quadratic non-residue (where z^((p-1)/2) = -1 (mod p))
		// since half of the numbers (mod p) are non-residues, we simply iterate through them
		uint64_t z = 2;
		while (is_quadratic_residue(z, prime)) z++;
		uint64_t M = S;
		uint64_t c = pow_mod(z, Q, prime);
		uint64_t t = pow_mod(N, Q, prime);
		uint64_t R = pow_mod(N, (Q+1)/2, prime);
		while (t > 1) {
			uint64_t t_exp = (t*t) % prime;
			uint64_t i = 1;
			while (t_exp != 1 && i < M) { t_exp = (t_exp*t_exp)%prime; i++; }
			if (i == M) return 0;
			uint64_t b = pow_mod(c, 1 << (M-i-1), prime);
			M = i;
			c = (b*b)%prime;
			t = (t*c)%prime;
			R = (R*b)%prime;
		}
		if (t == 1) return R;
		return 0;
	}

	// Returns a single solution to x^2 = N (mod p) (the other solution is x2 = p - x1)
	// If a solution is not found, zero is returned
	// p must be a prime for the algorithm to work
	template<int bits>
	uint64_t Tonelli_Shanks(int_limited<bits> N, uint64_t prime) {
		return Tonelli_Shanks(uint64_t(N%prime), prime);
	}

	// Returns a single solution to x^2 = N (mod p) (the other solution is x2 = p - x1)
	// If a solution is not found, zero is returned
	// p must be a prime for the algorithm to work
	template<int bits>
	int_limited<bits> gcd(int_limited<bits> a, int_limited<bits> b) {
		if (b == 0) return a;
		return gcd(b, a%b);
	}
}

