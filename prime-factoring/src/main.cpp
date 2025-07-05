#include "../../int_limited.hpp"
#include "factoriser.hpp"

int main() {
	using largeNumberLibrary::int_limited;
	factoriser_QS<256> QS(true);
	int_limited<256> a = 10000000001;
	QS.quadratic_sieve(a);

	std::vector<int_limited<256>> factors = Factoriser::factorise(a);
}