#include <cstdint>
#include <iostream>
#include <string>
#include <cmath>
#include <bitset>
#include <vector>
// #include <boost/multiprecision/cpp_int.hpp>
#include <chrono>

class bigInt {

};

// equivalent to INT64_MAX
constexpr uint64_t UINT63_MAX = 0x7FFFFFFFFFFFFFFF;
// equivalent to INT64_MIN
constexpr uint64_t BIT64_ON = 0x8000000000000000;

// Two's complement
class int128 {
	private: 
		// byte/word 0 and 1
		uint64_t B0;
		uint64_t B1;
		const int maxCharSize = ceil(128*log10(2));

	public:
		/*
		SECTION: CONSTRUCTION
		=============================================================
		default constructor DONE
		base constructor DONE
		IMPLICIT conversion FROM:
			uint64_t DONE
			int64_t DONE
			int DONE
		conversion TO:
			uint64_t DONE
			int64_t DONE
			int DONE
		= (assignment operator) DONE
		=============================================================
		*/


		int128() { B1 = 0; B0 = 0; }
		// Accepts most significant word first
		int128(uint64_t a, uint64_t b) {
			B1 = a;
			B0 = b;
		}
		int128(uint64_t a) {
			B1 = 0;
			B0 = a;
		}
		int128(int64_t a) {
			if (a < 0) {
				B1 = UINT64_MAX;
				B0 = UINT64_MAX + a + 1;
			} else {
				B1 = 0;
				B0 = a;
			}
		}
		int128(int a) {
			B1 = 0;
			B0 = a;
			if (a < 0) {
				B1 = UINT64_MAX;
				B0 += UINT64_MAX + 1;
			}
		}

		explicit operator uint64_t() const {
			if (B1 >= BIT64_ON ) return 0;
			if (B1 > 0) return UINT64_MAX;
			return B0;
		}
		explicit operator int64_t() const {
			int128 thisCopy = int128(B1, B0);
			if (thisCopy >= (int128)INT64_MAX) return INT64_MAX;
			if (thisCopy <= (int128)INT64_MIN) return INT64_MIN;
			// if B & BIT64_ON, then it is negative (and in two's complement)
			// otherwise it can't be on, so just return it
			return B0;
		}
		explicit operator int() const {
			int128 thisCopy = int128(B1, B0);
			if (thisCopy >= (int128)INT32_MAX) return INT32_MAX;
			if (thisCopy <= (int128)INT32_MIN) return INT32_MIN;
			// if B & BIT64_ON, then it is negative (and in two's complement)
			// otherwise it can't be on, so just return it
			return B0;
		}

		int128& operator= (int128 const& rhs) {
			B1 = rhs.B1;
			B0 = rhs.B0;
			return *this;
		}
		

		/*
		SECTION: PRINTING
		=============================================================
		toString
		<< (insertion to stream)
		>> (extraction from stream)
		=============================================================
		*/

		std::string toString() {
			char* s = new char[maxCharSize];
			char carry = 0;
			return "";
		}

		friend std::ostream& operator<<(std::ostream& os, int128 const& num) {
			os << std::bitset<64>(num.B1) << " " << std::bitset<64>(num.B0);
			return os;
		}

		/*
		SECTION: MATHEMATICAL FUNCTIONS
		=============================================================
		toPow();
		=============================================================
		*/


		/*
		SECTION: ARITHMETIC
		=============================================================
		+ (addition) DONE
		- (subtraction) DONE
		* (multiplication) DONE
		/ (division) DONE
		% (modulus)
		respective compound operators (+=, -=, *=, /=, %=)
		=============================================================
		*/


		int128& operator+=(int128 const& rhs) {
			// flags for overflow condition
			char flag1 = (B0 >= BIT64_ON) + (rhs.B0 >= BIT64_ON);
			B0 += rhs.B0;
			bool flag2 = B0 < BIT64_ON;

			// if either both had BIT64_ON, or only one had it on and the sum didn't
			if (flag1 + flag2 > 1) {
				B1 += 1;
			}
			B1 += rhs.B1;
			return *this;
		}
		int128 operator+(int128 const& rhs) {
			int128 result(B1, B0);
			return result += rhs;
		}

		int128& operator-=(int128 rhs) {
			return *this += ~rhs;
		}
		int128 operator-(int128 rhs) {
			int128 result(B1, B0);
			return result += ~rhs;
		}
		// negates -a
		int128 operator-() {
			int128 result(B1, B0);
			return ~result;
		}

		int128& operator*=(int128 rhs) {
			int128 multiplicand(B1, B0);
			// sets sign bit
			bool sign = (B1 ^ rhs.B1) >= BIT64_ON;
			if (multiplicand < 0) multiplicand = ~multiplicand;
			if (rhs < 0) rhs = ~rhs;
			B1 = multiplicand.B1 * rhs.B0;
			B1 += multiplicand.B0 * rhs.B1;
			B0 = 0;

			int counter = 0;
			while (rhs.B0 != 0) {
				if (rhs.B0%2 == 1) {
					*this += (multiplicand << counter);
				}
				counter++;
				rhs.B0 >>= 1;
			}
			
			if (sign) *this = ~*this;
			return *this;
		}
		int128 operator*(int128 const& rhs) {
			int128 result(B1, B0);
			return result *= rhs;
		}

		int128& operator/=(int128 rhs) {
			if (rhs == 0) throw std::domain_error("Divide by zero exception");
			int128 dividend(B1, B0);
			int128 divisor(rhs.B1, rhs.B0);
			// sets sign bit
			bool sign = (B1 ^ rhs.B1) >= BIT64_ON;
			B1 = 0;
			B0 = 0;
			// convert all to positive (even rhs for comparison)
			if (dividend < 0) dividend = ~dividend;
			if (divisor < 0) divisor = ~divisor;
			if (rhs < 0) rhs = ~rhs;

			if (rhs.B1 == 0) {
				B1 = dividend.B1/rhs.B0;
				B0 = dividend.B0/rhs.B0;
				dividend.B1 %= rhs.B0;
				dividend.B0 = 0;
			} else {
				dividend.B0 = 0;
			}

			// shift divisor maximum to the left
			int counter = 0;
			while ((divisor > 0) && (divisor <= dividend)) {
				counter++;
				divisor <<= 1;
			};

			while (divisor > rhs) {
				counter--;
				divisor >>= 1;
				if (divisor <= dividend) {
					*this += ((int128)1 << counter);
					dividend -= divisor;
				}
			}
			// at this point divisor is its original size
			*this += (divisor == dividend);
			
			if (sign) *this = ~*this;
			return *this;
		}
		int128 operator/(int128 const& rhs) {
			int128 result(B1, B0);
			return result /= rhs;
		}

		int128& operator%=(int128 rhs) {
			if (rhs == 0) throw std::domain_error("Divide by zero exception");

			int128 dividend(B1, B0);
			int128 divisor(rhs.B1, rhs.B0);
			// sets sign bit
			bool sign = B1 >= BIT64_ON;
			// convert all to positive (even rhs for comparison)
			if (dividend < 0) dividend = ~dividend;
			if (divisor < 0) divisor = ~divisor;
			if (rhs < 0) rhs = ~rhs;

			// shift divisor maximum to the left
			int counter = 0;
			while ((divisor > 0) && (divisor <= dividend)) {
				counter++;
				divisor <<= 1;
			};

			while (divisor > rhs) {
				counter--;
				divisor >>= 1;
				if (divisor <= dividend) {
					dividend -= divisor;
				}
			}
			// at this point divisor is its original size
			*this = dividend;
			
			if (sign) *this = ~*this;
			return *this;
		}
		int128 operator%(int128 const& rhs) {
			int128 result(B1, B0);
			return result %= rhs;
		}

		/*
		SECTION: BITWISE OPERATORS
		=============================================================
		^ (XOR) DONE
		| (OR) DONE
		& (AND) DONE
		~ (complement) DONE
		<< (shift left) DONE
		>> (shift right) DONE
		respective compound operators (^=, |=, &=, <<=, >>=) DONE
		=============================================================
		*/


		int128& operator^= (int128 const& rhs) {
			B1 ^= rhs.B1;
			B0 ^= rhs.B0;
			return *this;
		}
		int128 operator^ (int128 const& rhs) {
			int128 result(B1 ^ rhs.B1, B0 ^ rhs.B0);
			return result;
		}

		int128& operator|= (int128 const& rhs) {
			B1 |= rhs.B1;
			B0 |= rhs.B0;
			return *this;
		}
		int128 operator| (int128 const& rhs) {
			int128 result(B1 | rhs.B1, B0 | rhs.B0);
			return result;
		}

		int128& operator&= (int128 const& rhs) {
			B1 &= rhs.B1;
			B0 &= rhs.B0;
			return *this;
		}
		int128 operator& (int128 const& rhs) {
			int128 result(B1 & rhs.B1, B0 & rhs.B0);
			return result;
		}

		// returns two's complement
		int128 operator~ () {
			int128 result(~B1, ~B0);
			if (result.B0 == UINT64_MAX) result.B1 += 1;
			result.B0 += 1;
			return result;
		}

		int128& operator<<= (int const& rhs) {
			// special case, since for some reason the final `else` doesn't work
			if (rhs == 0) return *this;

			if (rhs >= 128) {
				B0 = 0;	
				B1 = 0;
			}
			else if (rhs >= 64) {
				B1 = B0 << (rhs-64);
				B0 = 0;
			} else {
				uint64_t temp = B1;
				B1 = B0 >> (64-rhs);
				B0 <<= rhs;
				B1 += temp << rhs;
			}
			return *this;
		}
		int128 operator<< (int const& rhs) {
			int128 result(B1, B0);
			return result <<= rhs;
		}

		int128& operator>>= (int const& rhs) {
			// special case, since for some reason the final `else` doesn't work
			if (rhs == 0) return *this;

			if (rhs >= 128) {
				B0 = 0;	
				B1 = 0;
			}
			else if (rhs >= 64) {
				B0 = B1 >> (rhs-64);
				B1 = 0;
			} else {
				uint64_t temp = B0;
				B0 = B1 << (64-rhs);
				B1 >>= rhs;
				B0 += temp >> rhs;
			}
			return *this;
		}
		int128 operator>> (int const& rhs) {
			int128 result(B1, B0);
			return result >>= rhs;
		}

		/*
		SECTION: RELATIONAL OPERATORS
		=============================================================
		== (equality) DONE
		!= (inequality) DONE
		> (greater-than) DONE
		< (less-than) DONE
		>= (greater-than-or-equal-to) DONE
		<= (less-than-or-equal-to) DONE
		=============================================================
		*/
		bool operator== (int128 const& rhs) {
			return B0 == rhs.B0 && B1 == rhs.B1;
		}
		bool operator!= (int128 const& rhs) {
			return B0 != rhs.B0 || B1 != rhs.B1;
		}
		bool operator> (int128 const& rhs) {
			// if different signs - false if B1 is negative, true if rhs.B1 is negative
			if ((B1 & BIT64_ON) != (rhs.B1 & BIT64_ON)) return B1 < rhs.B1;
			if (B1 == rhs.B1) {
				return B0 > rhs.B0;
			}
			return B1 > rhs.B0;
		}
		bool operator< (int128 const& rhs) {
			// if different signs - false if B1 is negative, true if rhs.B1 is negative
			if ((B1 & BIT64_ON) != (rhs.B1 & BIT64_ON)) return B1 > rhs.B1;
			if (B1 == rhs.B1) {
				return B0 < rhs.B0;
			}
			return B1 < rhs.B1;
		}
		bool operator>= (int128 const& rhs) {
			// if different signs - false if B1 is negative, true if rhs.B1 is negative
			if ((B1 & BIT64_ON) != (rhs.B1 & BIT64_ON)) return B1 < rhs.B1;
			if (B1 == rhs.B1) {
				return B0 >= rhs.B0;
			}
			return B1 >= rhs.B0;
		}
		bool operator<= (int128 const& rhs) {
			// if different signs - false if B1 is negative, true if rhs.B1 is negative
			if ((B1 & BIT64_ON) != (rhs.B1 & BIT64_ON)) return B1 > rhs.B1;
			if (B1 == rhs.B1) {
				return B0 <= rhs.B0;
			}
			return B1 <= rhs.B1;
		}

		/*
		SECTION: LOGICAL OPERATORS
		=============================================================
		! (NOT) DONE
		&& (AND) DONE
		|| (OR) DONE
		> When overloaded, these operators get function call precedence,
		and this short circuit behavior is lost
		This shouldn't be a problem here, since the overload itself makes use
		of normal boolean && and ||
		=============================================================
		*/
		bool operator! () {
			return B1 == 0 && B0 == 0;
		}
		bool operator&& (int128 const& rhs) {
			return (B1 != 0 || B0 != 0) && (rhs.B1 != 0 || rhs.B0 != 0);
		}
		bool operator|| (int128 const& rhs) {
			return B1 != 0 || B0 != 0 || rhs.B1 != 0 || rhs.B0 != 0;
		}
		
};


int main() {
	int128 x = int128(0, 0x37a991cef710a37b);

	std::vector<int128> a = {
		// INT64_MAX,
		// INT32_MAX,
		// INT16_MAX,
		19, 17, 16, 13, 12, 10, 8, 3, 2, 1,
		0,
		-1, -2, -3, -8, -10, -12, -13, -16, -17, -19,
		// INT16_MIN,
		// INT32_MIN,
		// INT64_MIN,
	};
	std::vector<int64_t> b = {
		// INT64_MAX,
		// INT32_MAX,
		// INT16_MAX,
		19, 17, 16, 13, 12, 10, 8, 3, 2, 1,
		0,
		-1, -2, -3, -8, -10, -12, -13, -16, -17, -19,
		// INT16_MIN,
		// INT32_MIN,
		// INT64_MIN,
	};
	int64_t max = 5000;
	int64_t min = -5000;

	int128 f = 1000;
	int64_t g = 1000;

	auto start = std::chrono::steady_clock::now();
	for (int i = min; i < max; i++) {
		for (int j = min; j < max; j++) {
			int64_t x = g + j;
		}
	}
	auto end = std::chrono::steady_clock::now();
	auto elapsed = std::chrono::duration<double>(end-start);
	std::cout << abs((max-min)*(max-min)) << " int64_t additions: \t\t" << elapsed.count() << " seconds" << std::endl;

	start = std::chrono::steady_clock::now();
	for (int i = min; i < max; i++) {
		for (int j = min; j < max; j++) {
			int128 x = f + j;
		}
	}
	end = std::chrono::steady_clock::now();
	elapsed = std::chrono::duration<double>(end-start);
	std::cout << abs((max-min)*(max-min)) << " int128 additions: \t\t" << elapsed.count() << " seconds" << std::endl;

	start = std::chrono::steady_clock::now();
	for (int i = min; i < max; i++) {
		for (int j = min; j < max; j++) {
			int64_t x = g - j;
		}
	}
	end = std::chrono::steady_clock::now();
	elapsed = std::chrono::duration<double>(end-start);
	std::cout << abs((max-min)*(max-min)) << " int64_t subtractions: \t" << elapsed.count() << " seconds" << std::endl;

	start = std::chrono::steady_clock::now();
	for (int i = min; i < max; i++) {
		for (int j = min; j < max; j++) {
			int128 x = f - j;
		}
	}
	end = std::chrono::steady_clock::now();
	elapsed = std::chrono::duration<double>(end-start);
	std::cout << abs((max-min)*(max-min)) << " int128 subtractions: \t\t" << elapsed.count() << " seconds" << std::endl;

	start = std::chrono::steady_clock::now();
	for (int i = min; i < max; i++) {
		for (int j = min; j < max; j++) {
			int64_t x = g * j;
		}
	}
	end = std::chrono::steady_clock::now();
	elapsed = std::chrono::duration<double>(end-start);
	std::cout << abs((max-min)*(max-min)) << " int64_t multiplications: \t" << elapsed.count() << " seconds" << std::endl;

	start = std::chrono::steady_clock::now();
	for (int i = min; i < max; i++) {
		for (int j = min; j < max; j++) {
			int128 x = f * j;
		}
	}
	end = std::chrono::steady_clock::now();
	elapsed = std::chrono::duration<double>(end-start);
	std::cout << abs((max-min)*(max-min)) << " int128 multiplications: \t" << elapsed.count() << " seconds" << std::endl;

	start = std::chrono::steady_clock::now();
	for (int i = min; i < max; i++) {
		for (int j = min; j < max; j++) {
			if (j == 0) continue;
			int64_t x = g / j;
		}
	}
	end = std::chrono::steady_clock::now();
	elapsed = std::chrono::duration<double>(end-start);
	std::cout << "~" << abs((max-min)*(max-min)) << " int64_t divisions: \t\t" << elapsed.count() << " seconds" << std::endl;

	start = std::chrono::steady_clock::now();
	for (int i = min; i < max; i++) {
		for (int j = min; j < max; j++) {
			if (j == 0) continue;
			int128 x = f / j;
		}
	}
	end = std::chrono::steady_clock::now();
	elapsed = std::chrono::duration<double>(end-start);
	std::cout << "~" << abs((max-min)*(max-min)) << " int128 divisions: \t\t" << elapsed.count() << " seconds" << std::endl;

	start = std::chrono::steady_clock::now();
	for (int i = min; i < max; i++) {
		for (int j = min; j < max; j++) {
			if (j == 0) continue;
			int64_t x = g % j;
		}
	}
	end = std::chrono::steady_clock::now();
	elapsed = std::chrono::duration<double>(end-start);
	std::cout << "~" << abs((max-min)*(max-min)) << " int64_t modulo: \t\t" << elapsed.count() << " seconds" << std::endl;

	start = std::chrono::steady_clock::now();
	for (int i = min; i < max; i++) {
		for (int j = min; j < max; j++) {
			if (j == 0) continue;
			int128 x = f % j;
		}
	}
	end = std::chrono::steady_clock::now();
	elapsed = std::chrono::duration<double>(end-start);
	std::cout << "~" << abs((max-min)*(max-min)) << " int128 modulo: \t\t" << elapsed.count() << " seconds" << std::endl;

	// return 0;


	for (int i = min; i < max; i++) {
		for (int j = min; j < max; j++) {
			if ((int128)i+(int128)j != i+j) {
				std::cout << "FAILED: " << i << " + " << j << std::endl;
				std::cout << i+j << std::endl;
				std::cout << ((int128)(i+j)) << std::endl;
				return 0;
			}
		}
	}
	std::cout << "PASSED ADDITION FROM " << max << " TO " << min << std::endl;


	for (int i = min; i < max; i++) {
		for (int j = min; j < max; j++) {
			if ((int128)i-(int128)j != i-j) {
				std::cout << "FAILED: " << i << " - " << j << std::endl;
				std::cout << i-j << std::endl;
				std::cout << ((int128)(i-j)) << std::endl;
				return 0;
			}
		}
	}
	std::cout << "PASSED SUBTRACTION FROM " << max << " TO " << min << std::endl;


	for (int i = min; i < max; i++) {
		for (int j = min; j < max; j++) {
			if ((int128)i*(int128)j != i*j) {
				std::cout << "FAILED: " << i << " * " << j << std::endl;
				std::cout << i*j << std::endl;
				std::cout << ((int128)(i*j)) << std::endl;
				return 0;
			}
		}
	}
	std::cout << "PASSED MULTIPLICATION FROM " << max << " TO " << min << std::endl;


	for (int i = min; i < max; i++) {
		for (int j = min; j < max; j++) {
			if (j == 0) continue;
			if ((int128)i/(int128)j != i/j) {
				std::cout << "FAILED: " << i << " / " << j << std::endl;
				std::cout << i/j << std::endl;
				std::cout << ((int128)(i/j)) << std::endl;
				return 0;
			}
		}
	}
	std::cout << "PASSED DIVISION FROM " << max << " TO " << min << std::endl;

	for (int i = min; i < max; i++) {
		for (int j = min; j < max; j++) {
			if (j == 0) continue;
			if ((int128)i%(int128)j != i%j) {
				std::cout << "FAILED: " << i << " % " << j << std::endl;
				std::cout << i%j << std::endl;
				std::cout << ((int128)(i%j)) << std::endl;
				std::cout << ((int128)i%(int128)j) << std::endl;
				return 0;
			}
		}
	}
	std::cout << "PASSED MODULO FROM " << max << " TO " << min << std::endl;

}