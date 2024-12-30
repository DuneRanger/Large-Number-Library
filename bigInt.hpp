#include <cstdint>
#include <iostream>
#include <string>
#include <cmath>
#include <bitset>
#include <vector>


namespace customBigInt {
	class bigInt {

	};

	// equivalent to INT64_MAX
	constexpr uint64_t UINT63_MAX = 0x7FFFFFFFFFFFFFFF;
	// equivalent to INT64_MIN
	constexpr uint64_t BIT64_ON = 0x8000000000000000;

	// Two's complement
	class int128 {
		private: 
			// byte 0 and 1
			uint64_t B0;
			uint64_t B1;
			static const int maxCharSize = 39;

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
				if (a < 0) {
					B1 = UINT64_MAX;
					B0 = UINT64_MAX + a + 1;
				} else {
					B1 = 0;
					B0 = a;
				}
			}
			int128(unsigned int a) {
				B0 = a;
			}

			explicit operator uint64_t() const {
				return B0;
			}
			// Simply returns LSB, because I don't want to be like boost, where I can't actually find out the bits of a number easily
			explicit operator int64_t() const {
				return (int64_t)B0;
			}
			explicit operator int() const {
				return (int)B0;
			}
			explicit operator char() const {
				return (char)B0;
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

			inline static std::string className() {
				return "customBigInt::int128";
			}

			// Returns a string of the current value converted to the desired base
			// '-' is appended to the start, if the number is negative, irregardless of the base
			inline std::string toString(uint64_t base = 10) {
				// Approximate the largest number of possible words
				int binWordSize = 0;
				uint64_t base_copy = base;
				while (base_copy != 0) {
					base_copy >>= 1;
					binWordSize++;
				}
				// -1 to binWordSize to account for unfilled bits
				// +1 at the end to act as a ceil() for cases like base-8 (requires 42.66... words)
				int maxWordCount = 128/(binWordSize - 1) + 1;

				// Convert this into a vector of word-size chunks (yes, it is a bit wasteful for low bases)
				std::vector<uint64_t> words(maxWordCount, 0);
				int128 num = *this;
				bool sign = num < 0;
				int index = maxWordCount-1;
				if (sign) num = ~num+1;
				do {
					words[index] = (uint64_t)(num%base);
					num /= base;
					index--;
				} while (num != 0);

				// Convert the word-size chunks into the string
				std::string output = "";
				if (sign) output += '-';
				for (uint64_t word : words) {
					if (output.size() <= sign && word == 0) continue;
					if (base < 10) {
						output += '0' + (char)word;
					} else if (base < 37) { // the whole alphabet
						char charWord = '0' + (char)word;
						if (word > 9) charWord = 'A' + (char)word - 10;
						output += charWord;
					} else {
						output += std::to_string(word) + "_"; // + word divider for clarity
					}
				}
				return output;
			}

			// Note: This overload doesn't take int128&, because it would throw an error when printing a complex expression
			// For example (a * -1)
			// It also doesn't consider num as a const, because methods can't be called on consts (at least from my understanding of the error)
			friend std::ostream& operator<<(std::ostream& os, int128 num) {
				os << num.toString();
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
				return *this += (~rhs + 1);
			}
			int128 operator-(int128 rhs) {
				int128 result(B1, B0);
				return result += (~rhs + 1);
			}
			// negates -a
			int128 operator-() {
				int128 result(B1, B0);
				return (~result + 1);
			}

			int128& operator*=(int128 rhs) {
				int128 multiplicand(B1, B0);
				// sets sign bit
				bool sign = (B1 ^ rhs.B1) >= BIT64_ON;
				B1 = 0;
				B0 = 0;
				if (multiplicand < 0) multiplicand = ~multiplicand + 1;
				if (rhs < 0) rhs = ~rhs + 1;

				int counter = 0;
				while (rhs != 0) {
					if (rhs.B0%2 == 1) {
						*this += (multiplicand << counter);
					}
					counter++;
					rhs >>= 1;
				}
				
				if (sign) *this = ~*this + 1;
				return *this;
			}
			int128 operator*(int128 const& rhs) {
				int128 result(B1, B0);
				return result *= rhs;
			}

			// Division truncates towards zero (just like C and boost)
			int128& operator/=(int128 divisor) {
				if (divisor == 0) throw std::domain_error("Divide by zero exception");
				int128 dividend(B1, B0);
				// sets sign bit
				bool sign = (B1 ^ divisor.B1) >= BIT64_ON;
				B1 = 0;
				B0 = 0;
				// convert all to positive
				if (dividend < 0) dividend = ~dividend + 1;
				if (divisor < 0) divisor = ~divisor + 1;

				// shift divisor maximum to the left
				int counter = 0;
				// Also makes sure, that the divisor is always positive
				while (divisor < dividend && divisor.B1 < (BIT64_ON >> 1)) {
					counter++;
					divisor <<= 1;
				};

				while (counter > -1) {
					while (divisor <= dividend) {
						*this += ((int128)1 << counter);
						dividend -= divisor;
					}
					counter--;
					divisor >>= 1;
				}
				
				if (sign) *this = ~*this + 1;
				return *this;
			}
			int128 operator/(int128 const& rhs) {
				int128 result(B1, B0);
				return result /= rhs;
			}

			int128& operator%=(int128 divisor) {
				if (divisor == 0) throw std::domain_error("Divide by zero exception");

				int128 dividend(B1, B0);
				// sets sign bit
				bool sign = B1 >= BIT64_ON;
				// convert all to positive (even rhs for comparison)
				if (dividend < 0) dividend = ~dividend + 1;
				if (divisor < 0) divisor = ~divisor + 1;

				// shift divisor maximum to the left
				int counter = 0;
				// Also makes sure, that the divisor is always positive
				while (divisor <= dividend && divisor.B1 < (BIT64_ON >> 1)) {
					counter++;
					divisor <<= 1;
				};

				while (counter > -1) {
					while (divisor <= dividend) {
						dividend -= divisor;
					}
					counter--;
					divisor >>= 1;
				}
				// at this point divisor is its original size
				*this = dividend;
				
				if (sign) *this = ~*this + 1;
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

			// returns bit NOT
			int128 operator~ () {
				int128 result(~B1, ~B0);
				return result;
			}

			int128& operator<<= (int const& rhs) {
				// Special case, because bitshifting by the bitsize of an integer is undefined (and inconsistent) behaviour
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
				// Special case, because bitshifting by the bitsize of an integer is undefined (and inconsistent) behaviour
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
				return B1 > rhs.B1;
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
				return B1 >= rhs.B1;
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
}