#include "../../../int_limited.hpp"
#include "../../factoriser.hpp"
#include "../../factoriser_basic.hpp"
#include <chrono>
#include <iostream>
using largeNumberLibrary::int_limited;
constexpr int bit_size = 256;
typedef int_limited<bit_size> big_int;

bool str_is_num(std::string& str) {
	for (char c : str) {
		if (c < '0' || c > '9') return false;
	}
	return true;
}

void print_help() {
	std::cout << "-h (--help)\tDisplays this help message" << std::endl;
	std::cout << "-v [num] (--verbose)\tSets the verbosity of debug logs. Accepts values from 0(default) to 9, however the current highest setting is 3" << std::endl;
}

int main(int argc, const char* argv[]) {

	int verbosity = 0;
	big_int value = 0;

	// load all arguments from command line
	for (int i = 1; i < argc; i++) {
		std::string arg(argv[i]);
		if (arg == "-v" || arg == "--verbose") {
				if (++i >= argc) { verbosity = 1; break; }
				else {
					std::string num(argv[i]);
					if (!str_is_num(num) || num.size() > 1) throw std::logic_error("Error: invalid verbosity level");
					verbosity = num[0] - '0';
				}
		} else if (arg == "-h" || arg == "--help") {
			print_help();
			return 0;
		} else {
			// accept value from input
			if (!str_is_num(arg)) throw std::logic_error("Error: invalid value for factorisation");
			std::string max_val = big_int::MAX_VALUE().toString();
			if (arg.size() >= max_val.size() && arg > max_val) throw std::overflow_error("Error: Value is larger than a signed " + std::to_string(bit_size) + " integer");
			value = arg;
		
		}
	}
	if (verbosity > 0) Factoriser::debug = true;
	if (verbosity > 1) Factoriser::QS_debug = true;
	if (verbosity > 2) Factoriser::sieve_debug= true;

	factoriser_basic::prepare_primes();
	
	auto start = std::chrono::steady_clock::now();
	std::vector<big_int> factors = Factoriser::factorise(value);
	auto end = std::chrono::steady_clock::now();

	std::cout << "Factors: ";
	big_int test = 1;
	for (big_int factor : factors) {
		assert(factoriser_basic::is_prime(factor));
		std::cout << factor << " ";
		test *= factor;
	}
	assert(value == test);
	if (verbosity > 0) std::cout << "Factorisation took " << std::chrono::duration<double>(end - start).count() << " seconds" << std::endl;
	if (verbosity > 1) std::cout << "Factors were asserted to be prime and their product was correct" << std::endl;
}