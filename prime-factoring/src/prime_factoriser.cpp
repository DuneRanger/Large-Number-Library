#include "headers.hpp"
#include "basic_factoriser.cpp"
#include "quadratic_sieve.cpp"

class Factoriser {
	private:
	public:
		static std::vector<qs_int> factorise(qs_int value) {
			std::vector<qs_int> factors;
			std::vector<uint64_t> small_factors = trial_division(value);
			for (uint64_t prime : small_factors) {
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
	QS::QuadraticSieve test;
	uint64_t a = 0x2782b;
	// uint64_t a = 0x2782ba7de;
	// qs_int b = "411902941625262175430";
	test.factorise(227179);
	// qs_int b = a;
	// int max_bit = (250-64)/2;
	// for (int i = 0; i < 10; i++) {
	// 	test.factorise(b);
	// 	b <<= 2;
	// 	b ^= a;
	// 	std::cout << std::endl;
	// }
}