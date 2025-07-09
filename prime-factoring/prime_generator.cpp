#include <array>
#include <fstream>
#include <string>
#include <cstdint>

// High enough to get the first prime after 1 000 000
constexpr uint64_t max_val = 1000000 + 4;
const std::string out_path = "./src/primes.txt";

int main() {
	std::array<bool, max_val+1>* sieve = new std::array<bool, max_val+1>;
	for (uint64_t i = 0; i < max_val; i++) (*sieve)[i] = true;

	for (uint64_t i = 2; i < max_val; i++) {
		if ((*sieve)[i]) {
			for (uint64_t j = 2*i; j < max_val; j += i) (*sieve)[j] = false;
		}
	}
	std::ofstream output(out_path);
	for (uint64_t i = 2; i < max_val; i++) {
		if ((*sieve)[i]) output << i << '\n';
	}
	output.close();
}

