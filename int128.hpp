#pragma once
#include <cstdint>
#include <iostream>
#include <string>
#include <vector>

namespace largeNumberLibrary {
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
				uint DONE
			conversion TO:
				uint64_t DONE
				int64_t DONE
				int DONE
				uint DONE
				char DONE
			= (assignment operator) DONE
			=============================================================
			*/
			#pragma region


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
				} else {
					B1 = 0;
				}
				B0 = a;
			}
			int128(int a) {
				if (a < 0) {
					B1 = UINT64_MAX;
				} else {
					B1 = 0;
				}
				B0 = a;
			}
			int128(unsigned int a) {
				B1 = 0;
				B0 = a;
			}

			// All explicit conversions simply returns the bits for the given bit amount
			// For example the minimum value (in two's complement) converted to a int64_t will simply return 0
			explicit operator uint64_t() const {
				return B0;
			}
			// Simply returns LSB to allow for easier bit manipulation
			explicit operator int64_t() const {
				return (int64_t)B0;
			}
			explicit operator int() const {
				return (int)B0;
			}
			explicit operator unsigned int() const {
				return (unsigned int)B0;
			}
			explicit operator char() const {
				return (char)B0;
			}
			int128& operator= (int128 const& rhs) {
				B1 = rhs.B1;
				B0 = rhs.B0;
				return *this;
			}
			#pragma endregion

			/*
			SECTION: PRINTING
			=============================================================
			className DONE
			toString DONE
			<< (insertion to stream) DONE
			=============================================================
			*/
			#pragma region

			static std::string className() {
				return "largeNumberLibrary::int128";
			}

			// Returns a string of the current value converted to the desired base
			// '-' is appended to the start, if the number is negative, regardless of the numerical base
			// Base is limited to a single unsigned 64 bit integer
			std::string toString(uint64_t base = 10) {
				if (base == 0) throw std::out_of_range("Unable to convert value to base 0");
				// Calculate the bits each word in the numerical base will store
				int binWordSize = 0;
				uint64_t base_copy = base;
				while (base_copy != 0) {
					base_copy >>= 1;
					binWordSize++;
				}
				// Approximate the largest number of possible words in the numerical base
				// -1 to binWordSize to account for unfilled bits
				// +1 at the end to act as a ceil() for cases like base-8 (requires 42.66... words)
				int maxWordCount = 128/(binWordSize - 1) + 1;

				// Convert this int128 into a vector of base-word-size chunks (yes, it is a bit wasteful for low bases)
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

				// Convert the word-size chunks into the output string
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

			// Note: This overload doesn't take a reference int128&, because it would throw an error when printing a complex expression
			// For example (a * -1)
			// It also doesn't consider num as a const, because methods can't be called on consts (at least from my understanding of the error)
			friend std::ostream& operator<<(std::ostream& os, int128 num) {
				os << num.toString();
				return os;
			}
			#pragma endregion


			/*
			SECTION: ARITHMETIC
			=============================================================
			+ (addition) DONE
			- (subtraction) DONE
			* (multiplication) DONE
			/ (division) DONE
			% (modulus) DONE
			respective compound operators (+=, -=, *=, /=, %=) DONE
			=============================================================
			*/
			#pragma region

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
			// negates value
			int128 operator-() {
				int128 result(B1, B0);
				return (~result + 1);
			}

			// We save a bit of time by manually multiplying some parts that are sure to fit within one of the words
			int128& operator*=(int128 rhs) {
				int128 multiplicand(B1, B0);
				// We ignore B1*rhs.B1, because it completely overflows anyway
				B1 = multiplicand.B1 * rhs.B0;
				B1 += multiplicand.B0 * rhs.B1;
				
				// split multiplicand.B0 and rhs.B0 for Karatsuba's algorithm
				uint64_t lowM = (multiplicand.B0 & UINT32_MAX);
				uint64_t highM = (multiplicand.B0 >> 32);
				uint64_t lowR = (rhs.B0 & UINT32_MAX);
				uint64_t highR = (rhs.B0 >> 32);
				
				uint64_t sumM = lowM + highM;
				uint64_t sumR = lowR + highR;

				uint64_t z0 = lowM * lowR;
				uint64_t z2 = highM * highR;
				int128 z1 = (sumM & UINT32_MAX) * (sumR & UINT32_MAX);
				z1 += (sumM >> 32) * (sumR << 32);
				z1 += (sumR >> 32) * (sumM << 32);
				z1.B1 += (sumM >> 32) * (sumR >> 32);
				z1 -= z0;
				z1 -= z2;

				B1 += z2;
				B0 = z0;
				*this += (z1 << 32);

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

				if (divisor > dividend) return *this;
				if (divisor.B1 == 0 && dividend.B1 == 0) {
					B0 = dividend.B0 / divisor.B0;
					if (sign) *this = ~(*this) + 1;
					return *this;
				}
				// if the divisor only consists of one 32-bit word, then divide manually
				if (divisor < 0x100000000) {
					uint64_t div = divisor.B0;
					// curWord contains the remainder from the preceding step in the first 32 bits
					// and then the current word from the dividend in the last 32 bits
					uint64_t curWord = (dividend.B1 >> 32);
					B1 |= (curWord / div) << 32;
					curWord = ((curWord % div) << 32) | (dividend.B1 & UINT32_MAX);
					B1 |= curWord / div;
					curWord = ((curWord % div) << 32) | (dividend.B0 >> 32);
					B0 |= (curWord / div) << 32;
					curWord = ((curWord % div) << 32) | (dividend.B0 & UINT32_MAX);
					B0 |= curWord / div;

					if (sign) *this = ~(*this) + 1;
					return *this;
				}

				// This is an implementation of the division algorithm described
				// in pages 272-273 in Knuth's Art of Computer Programming - Volume 2
				// u is the dividend, v is the divisor, q is the quotient
				std::vector<uint32_t> u = {uint32_t(dividend.B0 & UINT32_MAX), uint32_t(dividend.B0 >> 32), uint32_t(dividend.B1 & UINT32_MAX), uint32_t(dividend.B1 >> 32), 0};
				std::vector<uint32_t> v = {uint32_t(divisor.B0 & UINT32_MAX), uint32_t(divisor.B0 >> 32), uint32_t(divisor.B1 & UINT32_MAX), uint32_t(divisor.B1 >> 32)};
				std::vector<uint32_t> q = {0, 0, 0, 0};
				int vInd = 3;
				int uInd = 4;
				while (v[vInd] == 0) vInd--;
				while (u[uInd] == 0) uInd--;
				
				// if v is less than 16 bits
				// then shift u and v by 16 for a more accurate quotient estimate
				// (this later requires at least 144 bit accuracy for u)
				if (v[vInd] < (1 << 16)) {
					// the original divisor is also shifted to help calculate the remainder faster
					divisor <<= 16;
					v[vInd] <<= 16;
					for (int curInd = vInd; curInd > 0; curInd--) {
						v[curInd] |= v[curInd - 1] >> 16;
						v[curInd - 1] <<= 16;
					}
					for (int curInd = 4; curInd > 0; curInd--) {
						u[curInd] |= u[curInd - 1] >> 16;
						u[curInd - 1] <<= 16;
					}
				}
				// uInd shouldn't be updated after the shift
				// Because the original value is required for j

				for (int j = uInd - vInd; j >= 0; j--) {
					uint64_t curDigits = ((uint64_t(u[j + vInd + 1]) << 32) | u[j + vInd]);
					if (uint64_t(v[vInd]) > curDigits) {
						q[j] = 0;
						continue;
					}
					// quotient estimate and remainder
					uint64_t qEst = curDigits / v[vInd];
					uint64_t rem = curDigits - qEst * v[vInd];
					if (qEst >= UINT32_MAX || 
					(qEst * v[vInd - 1]) > ((rem << 32) | u[j + vInd - 1])) {
						qEst--;
						rem += v[vInd];
						// repeat the test until it fails (rem > UINT32_MAX results in failure)
						while (rem <= UINT32_MAX) {
							if ((qEst * v[vInd - 1]) > ((rem << 32) | u[j + vInd - 1])) {
								qEst--;
								rem += v[vInd];
							} else {
								break;
							}
						}
					}
					
					int128 difference = divisor * qEst;
					// The first four 32 bit words are obtained from difference
					// the fifth word is calculated by itself, since it is outside of 128 bit precision
					std::vector<uint32_t> diff = {uint32_t(difference.B0 & UINT32_MAX), uint32_t(difference.B0 >> 32), uint32_t(difference.B1 & UINT32_MAX), uint32_t(difference.B1 >> 32), uint32_t(((divisor.B1 >> 32) *qEst) >> 32)};
					// subtract difference with borrow
					bool borrow = false;
					for (int curInd = 0; curInd <= vInd + 1; curInd++) {
						bool flag1 = diff[curInd] > u[j + curInd];
						bool flag2 = (diff[curInd] == u[j + curInd]) && borrow;
						u[j + curInd] -= diff[curInd] + borrow;
						borrow = flag1 || flag2;
					}
					q[j] = qEst;
					// If the result is negative, add the divisor back once
					if (borrow) {
						q[j]--;
						bool carry = false;
						for (int curInd = 0; curInd <= vInd; curInd++) {
							// INT32_MIN in this context is BIT32_ON
							char flag1 = (v[curInd] >= INT32_MIN) + (u[j + curInd] >= INT32_MIN);
							u[j + curInd] += v[curInd] + carry;
							bool flag2 = u[j + curInd] < INT32_MIN;
							carry = (flag1 + flag2) > 1;
						}
						u[j + vInd + 1] += carry;
					}
				}
				// Concatenate the result into an int128
				B1 = (uint64_t(q[3]) << 32) | q[2];
				B0 = (uint64_t(q[1]) << 32) | q[0];
				
				if (sign) *this = ~*this + 1;
				return *this;
			}
			int128 operator/(int128 const& rhs) {
				int128 result(B1, B0);
				return result /= rhs;
			}
			// Keeps the sign from the dividend (original value)
			// The sign of the divisor doesn't affect anything
			int128& operator%=(int128 divisor) {
				if (divisor == 0) throw std::domain_error("Divide by zero exception");
				int128 dividend(B1, B0);
				// sets sign bit
				bool sign = (B1 >= BIT64_ON);
				// convert all to positive
				if (dividend < 0) dividend = ~dividend + 1;
				if (divisor < 0) divisor = ~divisor + 1;

				if (divisor > dividend) {
					*this = dividend;
					if (sign) *this = ~(*this) + 1;
					return *this;
				}
				B1 = 0;
				// For the most part, this is all a copy of division
				// where we take the remainder instead of the quotient
				if (divisor.B1 == 0 && dividend.B1 == 0) {
					B0 = dividend.B0 % divisor.B0;
					if (sign) *this = ~(*this) + 1;
					return *this;
				}

				// if the divisor only consists of one 32-bit word, then divide manually
				if (divisor < 0x100000000) {
					uint64_t div = divisor.B0;
					// curWord contains the remainder from the preceding step in the first 32 bits
					// and then the current word from the dividend in the last 32 bits
					uint64_t curWord = (dividend.B1 >> 32);
					curWord = ((curWord % div) << 32) | (dividend.B1 & UINT32_MAX);
					curWord = ((curWord % div) << 32) | (dividend.B0 >> 32);
					curWord = ((curWord % div) << 32) | (dividend.B0 & UINT32_MAX);
					B0 = curWord % div;

					if (sign) *this = ~(*this) + 1;
					return *this;
				}

				// This is an implementation of the division algorithm described
				// in pages 272-273 in Knuth's Art of Computer Programming - Volume 2
				// u is the dividend, v is the divisor, the remainder will be in u at the end
				std::vector<uint32_t> u = {uint32_t(dividend.B0 & UINT32_MAX), uint32_t(dividend.B0 >> 32), uint32_t(dividend.B1 & UINT32_MAX), uint32_t(dividend.B1 >> 32), 0};
				std::vector<uint32_t> v = {uint32_t(divisor.B0 & UINT32_MAX), uint32_t(divisor.B0 >> 32), uint32_t(divisor.B1 & UINT32_MAX), uint32_t(divisor.B1 >> 32)};
				int uInd = 4;
				int vInd = 3;
				while (u[uInd] == 0) uInd--;
				while (v[vInd] == 0) vInd--;
				
				// if v is less than 16 bits
				// then shift u and v by 16 for a more accurate quotient estimate
				// (this later requires at least 144 bit accuracy for u)
				bool shifted = false;
				if (v[vInd] < (1 << 16)) {
					shifted = true;
					// the original divisor is also shifted to help calculate the remainder faster
					divisor <<= 16;
					v[vInd] <<= 16;
					for (int curInd = vInd; curInd > 0; curInd--) {
						v[curInd] |= v[curInd - 1] >> 16;
						v[curInd - 1] <<= 16;
					}
					for (int curInd = 4; curInd > 0; curInd--) {
						u[curInd] |= u[curInd - 1] >> 16;
						u[curInd - 1] <<= 16;
					}
				}
				// uInd shouldn't be updated after the shift
				// Because the original value is required for j
				
				for (int j = uInd - vInd; j >= 0; j--) {
					uint64_t curDigits = ((uint64_t(u[j + vInd + 1]) << 32) | u[j + vInd]);
					if (uint64_t(v[vInd]) > curDigits) {
						continue;
					}
					// quotient estimate and remainder
					uint64_t qEst = curDigits / v[vInd];
					uint64_t rem = curDigits - qEst * v[vInd];
					if (qEst > UINT32_MAX || 
					(qEst * v[vInd - 1]) > ((rem << 32) | u[j + vInd - 1])) {
						qEst--;
						rem += v[vInd];
						// repeat the test until it fails (rem > UINT32_MAX results in failure)
						while (rem <= UINT32_MAX) {
							if ((qEst * v[vInd - 1]) > ((rem << 32) | u[j + vInd - 1])) {
								qEst--;
								rem += v[vInd];
							} else {
								break;
							}
						}
					}
					
					int128 difference = divisor * qEst;
					// The first four 32 bit words are obtained from difference
					// the fifth word is calculated by itself, since it is outside of 128 bit precision
					std::vector<uint32_t> diff = {uint32_t(difference.B0 & UINT32_MAX), uint32_t(difference.B0 >> 32), uint32_t(difference.B1 & UINT32_MAX), uint32_t(difference.B1 >> 32), uint32_t(((divisor.B1 >> 32) *qEst) >> 32)};
					// subtract difference with borrow
					bool borrow = false;
					for (int curInd = 0; curInd <= vInd + 1; curInd++) {
						bool flag1 = diff[curInd] > u[j + curInd];
						bool flag2 = (diff[curInd] == u[j + curInd]) && borrow;
						u[j + curInd] -= diff[curInd] + borrow;
						borrow = flag1 || flag2;
					}
					// If the result is negative, add the divisor back once
					if (borrow) {
						bool carry = false;
						for (int curInd = 0; curInd <= vInd; curInd++) {
							// INT32_MIN in this context is BIT32_ON
							char flag1 = (v[curInd] >= INT32_MIN) + (u[j + curInd] >= INT32_MIN);
							u[j + curInd] += v[curInd] + carry;
							bool flag2 = u[j + curInd] < INT32_MIN;
							carry = (flag1 + flag2 > 1);
						}
						u[j + vInd + 1] += carry;
					}
				}
				if (shifted) {
					B1 = (uint64_t(u[4]) << 48) | (uint64_t(u[3]) << 16) | (uint64_t(u[2]) >> 16);
					B0 = (uint64_t(u[2]) << 48) | (uint64_t(u[1]) << 16) | (uint64_t(u[0]) >> 16);
				} else {
					B1 = (uint64_t(u[3]) << 32) | u[2];
					B0 = (uint64_t(u[1]) << 32) | u[0];
				}
				
				if (sign) *this = ~*this + 1;
				return *this;
			}
			int128 operator%(int128 const& rhs) {
				int128 result(B1, B0);
				return result %= rhs;
			}
			#pragma endregion

			/*
			SECTION: BITWISE OPERATORS
			=============================================================
			^ (XOR) DONE
			| (OR) DONE
			& (AND) DONE
			~ (NOT) DONE
			<< (shift left) DONE
			>> (shift right) DONE
			respective compound operators (^=, |=, &=, <<=, >>=) DONE
			=============================================================
			*/
			#pragma region

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

			// Returns the bit NOT, so adding 1 gets the two's complement
			int128 operator~ () {
				int128 result(~B1, ~B0);
				return result;
			}

			// Classic non-arithmetic bitshift
			int128& operator<<= (unsigned int const& rhs) {
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
			// Classic non-arithmetic bitshift
			int128 operator<< (unsigned int const& rhs) {
				int128 result(B1, B0);
				return result <<= rhs;
			}

			// Classic non-arithmetic bitshift
			int128& operator>>= (unsigned int const& rhs) {
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
			// Classic non-arithmetic bitshift
			int128 operator>> (unsigned int const& rhs) {
				int128 result(B1, B0);
				return result >>= rhs;
			}
			#pragma endregion

			/*
			SECTION: RELATIONAL OPERATORS
			=============================================================
			== (equality) DONE
			!= (not equality) DONE
			> (greater-than) DONE
			< (less-than) DONE
			>= (greater-than-or-equal-to) DONE
			<= (less-than-or-equal-to) DONE
			=============================================================
			*/
			#pragma region

			bool operator== (int128 const& rhs) {
				return (B0 == rhs.B0 && B1 == rhs.B1);
			}
			bool operator!= (int128 const& rhs) {
				return (B0 != rhs.B0 || B1 != rhs.B1);
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
			#pragma endregion

			/*
			SECTION: LOGICAL OPERATORS
			=============================================================
			! (NOT) DONE
			&& (AND) DONE
			|| (OR) DONE

			> When overloaded, these operators get function call precedence,
			and short circuit behavior is lost
			> This shouldn't be a problem here, since the overload itself makes use of normal boolean && and ||
			=============================================================
			*/
			bool operator! () {
				return (B1 == 0 && B0 == 0);
			}
			bool operator&& (int128 const& rhs) {
				return ((B1 != 0 || B0 != 0) && (rhs.B1 != 0 || rhs.B0 != 0));
			}
			bool operator|| (int128 const& rhs) {
				return (B1 != 0 || B0 != 0 || rhs.B1 != 0 || rhs.B0 != 0);
			}
	};
}