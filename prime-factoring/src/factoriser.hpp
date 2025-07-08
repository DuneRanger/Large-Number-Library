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

		std::vector<qs_int> factors;
		std::vector<uint64_t> small_factors = factoriser_basic::trial_division(value);
		for (uint64_t prime : small_factors) {
			value /= prime;
			factors.push_back(prime);
		}
		if (value == 1) return factors;

		factoriser_QS<bit_size> QS(debug);
		
		std::vector<qs_int> big_factors = QS.quadratic_sieve(value);
		for (qs_int& prime : big_factors) {
			value /= prime;
			factors.push_back(prime);
		}

		return factors;
	};
}