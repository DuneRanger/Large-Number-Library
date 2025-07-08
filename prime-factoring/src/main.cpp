#include "../../int_limited.hpp"
#include "factoriser.hpp"

int main() {
	using largeNumberLibrary::int_limited;
	factoriser_QS<256> QS(true);
	int_limited<256> a = "116575300957664735452709";
	int_limited<256> b = 0xde;
	Factoriser::debug = true;
	for (int i = 0; i < 150; i++) {
		// std::vector<int_limited<256>> factors = QS.quadratic_sieve(a);
		std::vector<int_limited<256>> factors = Factoriser::factorise(a);
		for (int_limited<256>& factor : factors) {
			std::cout << factor << " ";
		}
		a *= b;
		a ^= (a << 1) ^ (a >> 3);
		std::cout << std::endl;
	};

	// std::vector<int_limited<256>> factors = Factoriser::factorise(a);
}