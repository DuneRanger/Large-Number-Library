#pragma once
#include <cstdint>
#include "factoriser_math.hpp"
#include "factoriser_basic.hpp"
#include "factoriser_QS.hpp"
#include "../../int_limited.hpp"

namespace Factoriser {
	using largeNumberLibrary::int_limited;

	bool debug = false;
	bool QS_debug = false;
	bool sieve_debug = false;

	// Simple bubble sort, suitable for ascending subsequences
	template<int bit_size>
	void sort_factors(std::vector<int_limited<bit_size>>& factors) {
		for (int i = 1; i < factors.size(); i++) {
			if (factors[i] > factors[i-1]) continue;
			for (int j = i; j > 0; j--) {
				if (factors[j] > factors[j-1]) break;
				int_limited<bit_size> temp = factors[j];
				factors[j] = factors[j-1];
				factors[j-1] = temp;
			}
		}
	}

	template<int bit_size>
	std::vector<int_limited<bit_size>> factorise(int_limited<bit_size> value) {
		typedef int_limited<bit_size> qs_int;
		if (debug) std::cout << "=============== Factoriser input: " << value << " (" << value.ilog2() << " bits) ===============" << std::endl;
		if (factoriser_basic::is_prime(value)) return {value};
		
		std::vector<qs_int> factors;

		if (debug) std::cout << "Stage 1: trial division up to 1000 & primality test" << std::endl;
		{
			std::vector<uint64_t> small_factors = factoriser_basic::trial_division(value, 1000);
			for (uint64_t prime : small_factors) {
				value /= prime;
				factors.push_back(prime);
			}
			if (value == 1) return factors;
			if (factoriser_basic::is_prime(value)) {
				factors.push_back(value);
				return factors;
			}
		}

		if (debug) std::cout << "Stage 2: trial division up to 100000 & primality test" << std::endl;
		{
			std::vector<uint64_t> small_factors = factoriser_basic::trial_division(value, 100000);
			for (uint64_t prime : small_factors) {
				value /= prime;
				factors.push_back(prime);
			}
			if (value == 1) return factors;
			if (factoriser_basic::is_prime(value)) {
				factors.push_back(value);
				return factors;
			}
		}

		if (debug) std::cout << "Stage 3: Quadratic sieve until the value is a strong probable prime to 25 bases" << std::endl;
		{
			factoriser_QS<bit_size> QS(QS_debug, sieve_debug);
			
			do {
				if (QS_debug) std::cout << std::endl;
				std::vector<qs_int> big_factors = QS.quadratic_sieve(value);
				for (qs_int& prime : big_factors) {
					value /= prime;
					factors.push_back(prime);
				}
			} while (!factoriser_basic::is_prime(value));
			if (QS_debug) std::cout << std::endl;
		}
		
		// value should be either a strong probable prime or 1 at this point
		if (value != 1) factors.push_back(value);
		sort_factors(factors);
		return factors;
	};
}