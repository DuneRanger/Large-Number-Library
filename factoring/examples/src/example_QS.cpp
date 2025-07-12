#include "../../../int_limited.hpp"
#include "../../factoriser_QS.hpp"
#include "../../factoriser_basic.hpp"
#include <chrono>
#include <iostream>

int main() {
	using largeNumberLibrary::int_limited;
	bool QS_debug = true, sieve_debug = false;
	Factoriser::QuadraticSieve<256> QS(QS_debug, sieve_debug);
	int_limited<256> a, b;
	int test_iterations;
	b = 0xde;
	Factoriser::Basic::prepare_primes();

	// 32-167 bits | for quadratic sieve: 88-160 bits
	// a = 0xdeadbeef; test_iterations = 16;

	// 77-176 bits | for quadratic sieve: 77-170 bits
	a = "116575300957664735452709"; test_iterations = 12;

	// 140-176 bits | for quadratic sieve: 111 - 170 bits
	// a = "1388320592414173597917060811948270198768104"; test_iterations = 5;

	for (int i = 0; i < test_iterations; i++) {
		auto start = std::chrono::steady_clock::now();
		std::vector<int_limited<256>> factors = QS.factorise(a);
		auto end = std::chrono::steady_clock::now();

		std::cout << "Factors: ";
		int_limited<256> test = 1;
		for (int_limited<256> factor : factors) {
			assert(Factoriser::Basic::is_prime(factor));
			std::cout << factor << " ";
			test *= factor;
		}
		if (a == test) std::cout << "\nFactors were asserted to be prime and their product was correct\n";
		else std::cout  << "\nFactors were asserted to be prime however not all factors were found. Consider increasing relations gathered\n";
		std::cout << "Factorisation took " << std::chrono::duration<double>(end - start).count() << " seconds\n" << std::endl;

		a *= b;
		a ^= (a << 1) ^ (a >> 3);
	}
}