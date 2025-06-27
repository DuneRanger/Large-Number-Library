#include "headers.hpp"
#include "quadratic_sieve.cpp"
#include "basic_factoriser.cpp"

class Factoriser {
	private:
	public:
		static std::vector<qs_int> factorise(qs_int value) {
			std::vector<qs_int> factors;
			std::vector<uint32_t> small_factors = trial_division(value);
			for (uint32_t prime : small_factors) {
				value /= prime;
				factors.push_back(prime);
			}

			return factors;
		};
		static std::vector<qs_int> factorise(std::string value) {
			for (int i = 0; i < value.size(); i++) {
				if (value[i] < '0' || value[i] > '9') throw std::logic_error("Error: invalid numerical string for Factoriser::factorise");
			}
			std::string max_val = qs_int::MAX_VALUE().toString();
			if (value.size() > max_val.size()) throw std::logic_error("Error: string value cannot be represented by qs_int for Factoriser::factorise");
			for (int i = 0; i < value.size(); i++) {
				if (value[i] < max_val[i]) break;
				if (value[i] == max_val[i]) continue;
				throw std::logic_error("Error: string value cannot be represented by qs_int for Factoriser::factorise");
			}
			return factorise(qs_int(value));
		}
};



int main() {
	std::vector<qs_int> factors = Factoriser::factorise(0xDEADBEEF);
	// for (int i = 0; i < factors.size(); i++) {
	// 	std::cout << factors[i] << ", ";
	// }
	// std::cout << std::endl;
	QS::QuadraticSieve test;
	uint64_t a = 0x2782ba7de;
	qs_int b = a;
	for (int i = 0; i < 10; i++) {
		test.factorise(b);
		b <<= 2;
		b ^= a;
		std::cout << std::endl << std::endl;
	}
	// test.factorise("100000001");
	// std::cout << Tonelli_Shanks(5, 41) << std::endl;
	// std::cout << Tonelli_Shanks(100000001%37, 37) << std::endl;
}