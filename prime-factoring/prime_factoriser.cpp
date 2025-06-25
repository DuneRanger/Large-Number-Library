#include <vector>
#include <fstream>
#include <iostream>
#include "./quadratic_sieve.hpp"
#include "../int_limited.hpp"

class Factoriser {
	typedef largeNumberLibrary::int512 int512;

	private:
		std::vector<int512> factors;

		int512 trial_division(int512 value) {
			std::ifstream primes("./primes.txt");
			int prime;
			while (primes >> prime) {
				while (!(value % prime)) {
					factors.push_back(prime);
					value /= prime;
					if (value == 1) return;
				}
			}
		}
	public:
		static std::vector<int512> factorise(int512 value) {
			value = trial_division(value);

			return factors;
		};
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
};

int main() {
	std::vector<largeNumberLibrary::int512> factors = Factoriser::factorise(0xDEADBEEF);
	for (int i = 0; i < factors.size(); i++) {
		std::cout << factors[i] << ", ";
	}
	std::cout << std::endl;

}