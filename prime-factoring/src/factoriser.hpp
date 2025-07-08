#pragma once
#include <cstdint>
#include "factoriser_math.hpp"
#include "factoriser_basic.hpp"
#include "factoriser_QS.hpp"
#include "../../int_limited.hpp"

namespace Factoriser {
	using largeNumberLibrary::int_limited;

	bool debug = false;

	template<int bit_size>
	std::vector<int_limited<bit_size>> factorise(int_limited<bit_size> value) {
		typedef int_limited<bit_size> qs_int;
		if (debug) std::cout << "Factoriser input: " << value << std::endl;
		if (factoriser_basic::is_prime(value)) return {value};
		
		std::vector<qs_int> factors;

		// Stage 1: trial division up to 1000
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

		// Stage 2: trial division up to 1000000
		{
			std::vector<uint64_t> small_factors = factoriser_basic::trial_division(value);
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

		// Stage 3: quadratic sieve until value is a strong probable prime to 25 bases
		{
			factoriser_QS<bit_size> QS(debug);
			
			do {
				std::vector<qs_int> big_factors = QS.quadratic_sieve(value);
				for (qs_int& prime : big_factors) {
					value /= prime;
					factors.push_back(prime);
				}
			} while (0 || !factoriser_basic::is_prime(value));
		}
		
		// value should be either a strong probable prime or 1 at this point
		if (value != 1) factors.push_back(value);
		return factors;
	};
}