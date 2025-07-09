#include "../../int_limited.hpp"
#include "factoriser.hpp"
#include <chrono>
#include <iostream>

int main() {
	using largeNumberLibrary::int_limited;
	factoriser_QS<256> QS(true);
	Factoriser::debug = true;
	Factoriser::QS_debug = true;
	// Factoriser::sieve_debug = true;
	int_limited<256> a, b;
	// a = 0xdeadbeef;
	a = "116575300957664735452709";
	// a = "1388320592414173597917060811948270198768104"; // reduces into a 111 bit number made up of 3 large primes
	b = 0xde;

	for (int i = 0; i < 15; i++) {
		std::vector<int_limited<256>> factors = Factoriser::factorise(a);

		std::cout << "Factors: ";
		int_limited<256> test = 1;
		for (int_limited<256> factor : factors) {
			assert(factoriser_basic::is_prime(factor));
			std::cout << factor << " ";
			test *= factor;
		}
		assert(a == test);
		std::cout << "\nFactors were asserted to be prime and their product was correct\n\n" << std::endl;

		a *= b;
		a ^= (a << 1) ^ (a >> 3);
	};
}