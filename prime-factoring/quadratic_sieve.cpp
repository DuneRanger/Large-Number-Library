#pragma once
#include <vector>
#include "../int_limited.hpp"

// The following is a rudimentary implementation of the quadratic sieve algorithm, based on my limited understanding of how it works
class QuadraticSieve {
	typedef largeNumberLibrary::int512 int512;
	private:
		int512 N; // what we're trying to factorise
		int512 kN; // what we work with (modulo this)

		void prepare_kN() {}
		void prepare_polynomials() {}
		void sieve() {}
		void sieve() {}
	public:
		static std::vector<int512> factorise(int512 value) {
			
		}
		static std::vector<int512> factorise(std::string value) {
			for (int i = 0; i < value.size(); i++) {
				if (value < '0' || value > '9') throw std::logic_error("Error: invalid numerical string for Factoriser::factorise");
			}
			std::string max_val = int512::MAX_VALUE().toString();
			if (values.size() > max_val.size()) throw std::logic_error("Error: string value cannot be represented by int512 for Factoriser::factorise");
			for (int i = 0; i < value.size(); i++) {
				if (values[i] < max_val[i]) break;
				if (values[i] == max_val[i]) continue;
				throw std::logic_error("Error: string value cannot be represented by int512 for Factoriser::factorise");
			}
			return factorise(int512(value));
		}
}
