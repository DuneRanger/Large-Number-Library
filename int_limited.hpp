#pragma once
#include <cstdint>
#include <iostream>
#include <string>
#include <vector>
#include "./int128.hpp"


namespace customBigInt {

	// Two's complement
	// Constructs a vector of 64 bit unsigned integers, so that the specified bit size fits
	// If the bitSize isn't a multiple of 64, operations will still be processed for all 64 bits of the most significant word
	// However overflow will still occur if the value were to surpass the bitSize
	// Comparisons will also ignore any extra bits above the bitSize
	// No further optimizations are made on the most significant word (even if the instance only has 1 word)
	// All operations occur on class instances with equal bitSize
	template <int bitSize>
	class int_limited {
		private:
			const int wordCount = bitSize/64 + (bitSize%64 > 0);

			// LSb first
			// The most significant word is the last one
			std::vector<uint64_t> words = std::vector<uint64_t>(wordCount, 0);

			// Most and Least Significant Word containing a non-zero bit
			// Used for a minor optimization for arithmetic operations
			// Doesn't really help for small negative numbers
			int MSW = 0;
			int LSW = 0;

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
				char DONE
			= (assignment operator) DONE
			importBits (vector) DONE
			importBits (iterator to iterator) DONE
			exportBits DONE
			=============================================================
			*/
			#pragma region

			// Accepts conversions from individual standard int types (doesn't change MSW and LSW)
			// To convert multiple integers of a type into an int_limited
			// It is required to declare an instance and call importBits

			// For all of these instances, LSW and MSW are by default zero, which is correct;
			int_limited() {
				static_assert(bitSize > 1, "Invalid int_limited size");
			}
			int_limited(uint64_t a) {
				static_assert(bitSize > 1, "Invalid int_limited size");
				this->words[0] = a;
				this->truncateExtraBits();
			}
			int_limited(int64_t a) {
				static_assert(bitSize > 1, "Invalid int_limited size");
				if (a < 0) {
					for (int i = 1; i < this->wordCount; i++) {
						this->words[i] = UINT64_MAX;
					}
					this->updateMSW(this->wordCount - 1);
				}
				this->words[0] = a;
				this->truncateExtraBits();
			}
			int_limited(int a) {
				static_assert(bitSize > 1, "Invalid int_limited size");
				if (a < 0) {
					for (int i = 1; i < this->wordCount; i++) {
						this->words[i] = UINT64_MAX;
					}
					this->updateMSW(this->wordCount - 1);
				}
				this->words[0] = (int64_t)a;
				this->truncateExtraBits();
			}
			int_limited(unsigned int a) {
				static_assert(bitSize > 1, "Invalid int_limited size");
				this->words[0] = a;
				this->truncateExtraBits();
			}

			// All explicit conversions simply returns the bits for the given bit amount
			// For example the minimum value (in two's complement) converted to a int64_t will simply return 0
			explicit operator uint64_t() const {
				return this->words[0];
			}
			// Simply returns LSB to allow for easier bit manipulation
			explicit operator int64_t() const {
				return (int64_t)this->words[0];
			}
			explicit operator int() const {
				return (int)this->words[0];
			}
			explicit operator char() const {
				return (char)this->words[0];
			}

			int_limited& operator= (int_limited const& rhs) {
				for (int i = 0; i < rhs.wordCount; i++) {
					this->words[i] = rhs.words[i];
				}
				this->updateLSW(rhs.LSW);
				this->updateMSW(rhs.MSW);
				return *this;
			}

			// For simplicity's sake this function only accepts
			// a vector of unsigned 64 bit integers from the standard library
			void importBits(std::vector<uint64_t>& newWords) {
				*this = 0;
				for (int i = 0; i < this->wordCount && i < newWords.size(); i++) {
					this->words[i] = newWords[i];
				}
				this->updateLSW(0);
				this->updateMSW(newWords.size()-1);
				return;
			}

			// Starts importing from newWords[startIndex] (inclusive) to newWords[endIndex - 1]
			// Import into the destinations words, starting from wordOffset
			void importBits(std::vector<uint64_t>& newWords, int startIndex, int endIndex, int wordOffset = 0) {
				*this = 0;
				int maxWord = std::min((int)newWords.size(), endIndex);
				maxWord = std::min(maxWord, startIndex + (this->wordCount - wordOffset));
				for (int i = startIndex; i < maxWord; i++) {
					this->words[wordOffset] = newWords[i];
					wordOffset++;
				}
				this->updateLSW(0);
				this->updateMSW(endIndex - startIndex + wordOffset + 1);
				return;
			}

			// The first iterator is taken as the Least Significant Word (+ wordOffset)
			// The second iterator is non-inclusive in the import
			void importBits(std::vector<uint64_t>::iterator beginWords, std::vector<uint64_t>::iterator endWords, int wordOffset = 0) {
				*this = 0;
				int index = wordOffset;
				while (beginWords != endWords) {
					this->words[index] = *beginWords;
					beginWords++;
					index++;
				}
				this->updateLSW(wordOffset);
				this->updateMSW(index - 1);
				return;
			}

			// For simplicity's sake this function only returns
			// a vector of unsigned 64 bit integers from the standard library
			// Currently returns *all* words, even those higher than the Most Significant Word
			std::vector<uint64_t> exportBits() {
				return this->words;
			}
			#pragma endregion

			/*
			SECTION: HELPER FUNCTIONS
			=============================================================
			Truncate extra bits DONE
			Update LSW DONE
			Update MSW DONE
			Word Shift Left DONE
			Word Shift Right DONE
			Bit Shift Left DONE
			Bit Shift Right DONE
			Simple Multiplication DONE
			=============================================================
			*/
			#pragma region

			void truncateExtraBits() {
				int bitsInMSW = bitSize % 64;
				if (bitsInMSW == 0) return;
				this->words[this->wordCount-1] &= UINT64_MAX >> (64 - bitsInMSW);
				if (this->words[this->wordCount-1] == 0 && this->wordCount > 1) this->updateMSW(this->MSW);
				return;
			}

			void updateLSW(int lowerBound) {
				lowerBound = std::max(0, lowerBound);
				// Find the highest non-zero word
				while (this->words[lowerBound] == 0) {
					lowerBound++;
				}
				// If the number is zero
				if (lowerBound >= this->wordCount) lowerBound = 0;
				this->LSW = lowerBound;
				return;
			}

			void updateMSW(int upperBound) {
				upperBound = std::min(this->wordCount - 1, upperBound);
				// Find the highest non-zero word
				while (this->words[upperBound] == 0) {
					upperBound--;
				}
				// If the number is zero
				if (upperBound < 0) upperBound = 0;
				this->MSW = upperBound;
				if (this->MSW == this->wordCount - 1) {
					this->truncateExtraBits();
				}
				return;
			}
			
			// Shift a whole word left by an amount of words
			void wordShiftLeft(int wordIndex, int shift) {
				if (shift == 0) return;
				if (wordIndex + shift < this->wordCount) {
					this->words[wordIndex + shift] = this->words[wordIndex];
				}
				this->words[wordIndex] = 0;
				return;
			}

			// Shift a whole word right by an amount of words
			void wordShiftRight(int wordIndex, int shift) {
				if (shift == 0) return;
				if (wordIndex - shift > -1) {
					this->words[wordIndex - shift] = this->words[wordIndex];
				}
				this->words[wordIndex] = 0;
				return;
			}

			// Shifts a whole word left by an amount of bits
			void bitShiftLeft(int wordIndex, int shift) {
				if (shift == 0) return;
				if (wordIndex + 1 < this->wordCount) {
					this->words[wordIndex + 1] |= this->words[wordIndex] >> (64 - shift);
				}
				this->words[wordIndex] <<= shift;
				return;
			}

			// Shifts a whole word right by an amount of bits
			void bitShiftRight(int wordIndex, int shift) {
				if (shift == 0) return;
				if (wordIndex - 1 > -1) {
					this->words[wordIndex - 1] |= this->words[wordIndex] << (64 - shift);
				}
				this->words[wordIndex] >>= shift;
				return;
			}

			int_limited& basicMult(int_limited const& A, int_limited const& B) {
				*this = 0;
				for (int b_i = B.LSW; b_i <= B.MSW; b_i++) {
					uint64_t carry = 0;
					bool secondCarry = false;
					for (int a_i = A.LSW; a_i <= A.MSW; a_i++) {
						// If the result is outside of precision, continue
						if (a_i + b_i >= this->wordCount) continue;
						// If some of the lower 64 bits of the result fit into the integer
						// Then multiply with only 64 bit precision (and additional bits for the carry)
						if (a_i + b_i + 1 == this->wordCount) {
						// std::cout << std::bitset<64>(this->words[2]) << std::endl;
							this->words[a_i + b_i] +=  A.words[a_i] * B.words[b_i] + carry + secondCarry;
							// No need to set carry, because it will be out of precision next iteration
							continue;
						}
						// product cannot overflow, unless carry = (UINT64_MAX << 1) + 1
						int128 product = carry;
						product += secondCarry;
						int128 multiplicand = A.words[a_i];
						uint64_t multiplier = B.words[b_i];
						while (multiplier != 0) {
							// this slightly speeds up the loop
							// if (multiplier%4 == 0) {
							// 	multiplicand <<= 2;
							// 	multiplier >>= 2;
							// 	continue;
							// }
							if (multiplier%2 == 1) {
								product += multiplicand;
							}
							multiplicand <<= 1;
							multiplier >>= 1;
						}
						char flag1 = (this->words[a_i + b_i] >= BIT64_ON) + ((uint64_t)product >= BIT64_ON);
						this->words[a_i + b_i] += (uint64_t)product;
						bool flag2 = this->words[a_i + b_i] < BIT64_ON;
						// The maximum value of (product >> 64) is UINT64_MAX, which means that
						// by itself, it can fit in the carry, but (flag1 && flag2) doesn't have to
						// That is why we separate them
						carry = (uint64_t)(product >> 64);
						secondCarry = (flag1 + flag2) > 1;
					}

					if (b_i + A.MSW + 1 < this->wordCount) {
						// if (carry + secondCarry) overflows and will fit into precision, then overflow
						if (secondCarry && (carry == UINT64_MAX) && ((b_i + A.MSW + 2) < this->wordCount)) this->words[b_i + A.MSW + 2] = 1;
						// Otherwise simply add regardless of overflow/precision
						else this->words[b_i + A.MSW + 1] = carry + secondCarry;
					}

					// No need to worry about checking for a new carry, because [b_i + A.MSW + 1] is guaranteed to have been empty at this point
				}
				this->updateLSW(A.LSW);
				this->updateMSW(A.MSW + B.MSW + 1);
				return *this;
			}
			#pragma endregion

			/*
			SECTION: PRINTING
			=============================================================
			toString DONE
			<< (insertion to stream) DONE
			>> (extraction from stream)
			=============================================================
			*/
			#pragma region

			static std::string className() {
				return std::to_string(bitSize) + " bit customBigInt::int_limited";
			}

			// Returns a string of the current value converted to the desired base
			// '-' is appended to the start, if the number is negative, irregardless of the base
			// Base is limited to a single unsigned 64 bit integer
			std::string toString(uint64_t base = 10) {
				// Approximate of the largest number of possible words in the chosen base
				int binWordSize = 0;
				uint64_t base_copy = base;
				while (base_copy != 0) {
					base_copy >>= 1;
					binWordSize++;
				}
				// -1 to binWordSize to account for unfilled bits
				// +1 at the end to act as a ceil() for special cases
				int maxWordCount = bitSize/(binWordSize - 1) + 1;

				// Convert this into a vector of uin64_t chunks (a bit wasteful for low bases)
				std::vector<uint64_t> baseWords(maxWordCount, 0);
				int_limited num = *this;
				bool sign = num < 0;
				if (sign) num = ~num+1;
				// baseWords will be in MSb first, for ease of conversion to a string
				int index = maxWordCount-1;
				do {
					baseWords[index] = (uint64_t)(num%base);
					num /= base;
					index--;
				} while (num != 0);
				// Convert the word-size chunks into the string
				std::string output = "";
				if (sign) output += '-';
				for (uint64_t word : baseWords) {
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

			// Note: This overload doesn't take a reference, because it would throw an error when printing a complex expression
			// For example (a * -1)
			// It also doesn't consider num as a const, because methods can't be called on consts (at least from my understanding of the error)
			friend std::ostream& operator<<(std::ostream& os, int_limited num) {
				os << num.toString();
				return os;
			}
			#pragma endregion

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
			% (modulus) DONE
			respective compound operators (+=, -=, *=, /=, %=) DONE
			=============================================================
			*/
			#pragma region
			
			int_limited& operator+= (int_limited const& rhs) {
				bool carry = false;
				// Cycle from the lowest word in rhs with a non-zero value
				for (int i = rhs.LSW; i <= rhs.MSW; i++) {
					char flag1 = (this->words[i] >= BIT64_ON) + (rhs.words[i] >= BIT64_ON);
					this->words[i] += rhs.words[i];
					this->words[i] += carry;
					bool flag2 = this->words[i] < BIT64_ON;
					// carry = (either both had BIT64_ON), or (only one had it on and the sum didn't)
					carry = (flag1 + flag2 > 1);
				}
				// Finish off adding the carry to other words in *this
				for (int i = rhs.MSW + 1; i < this->wordCount; i++) {
					bool flag1 = this->words[i] >= BIT64_ON;
					this->words[i] += carry;
					bool flag2 = this->words[i] < BIT64_ON;
					
					if (flag1 && flag2) carry = true;
					else break;
				}

				this->updateLSW(std::min(this->LSW, rhs.LSW));
				this->updateMSW(std::max(this->MSW, rhs.MSW) + 1); // +1 for potential carry
				return *this;
			}
			int_limited operator+ (int_limited const& rhs) {
				int_limited result = *this;
				return result += rhs;
			}

			int_limited& operator-= (int_limited rhs) {
				return *this += (~rhs + 1);
			}
			int_limited operator- (int_limited rhs) {
				int_limited result = *this;
				return result += (~rhs + 1);
			}
			// negates value
			int_limited operator- () {
				int_limited result = *this;
				return (~result + 1);
			}

			// Many thanks to the author of http://kt8216.unixcab.org/karatsuba/index.html
			// The website cleared many doubts I had about my own implementation and helped guide me in the right direction

			// Multiplication done by Karatsuba's algorithm
			// Design changes and decisions heavily influenced by http://kt8216.unixcab.org/karatsuba/index.html
			int_limited& operator*= (int_limited const& rhs) {
				int_limited A, B;
				if (this->MSW < rhs.MSW) {
					A = rhs;
					B = *this;
				} else {
					A = *this;
					B = rhs;
				}
				if (B.MSW <= 8) {
					*this = this->basicMult(A, B);
					return *this;
				}
				int maxWordAmount = A.MSW;
				int splitWordIndex  = maxWordAmount / 2 + 1;
				int_limited lowA = 0;
				lowA.importBits(A.words, 0, splitWordIndex);
				int_limited highA = 0;
				highA.importBits(A.words, splitWordIndex, maxWordAmount + 1);

				if (splitWordIndex >= B.MSW) {
					*this = ((highA * B) << (64*splitWordIndex)) + (lowA * B);
					return *this;
				}

				int_limited lowB = 0;
				lowB.importBits(B.words, 0, splitWordIndex);
				int_limited highB = 0;
				highB.importBits(B.words, splitWordIndex, maxWordAmount + 1);

				int_limited z0 = lowA * lowB;
				int_limited z2 = highA * highB;
				int_limited z1 = (lowA + highA) * (highB + lowB) - z0 - z2;

				*this = z2;
				*this <<= 64*splitWordIndex;
				*this += z1;
				*this <<= 64*splitWordIndex;
				*this += z0;
				this->updateLSW(A.LSW);
				this->updateMSW(A.MSW + B.MSW + 1);
				return *this;
			}
			int_limited operator* (int_limited const& rhs) {
				int_limited result = *this;
				return result *= rhs;
			}

			// Any sort of optimized division algorithm depend on optimized multiplication algorithms
			// And frankly I don't completely believe that the constant of my multiplication implementation
			// are good enough for my implementation of a simple division optimization (Newton-Raphson method)
			// to be significantly faster than a "O(n^2)" implementationdone with MSW, LSW and bitshifts

			// Considering the implementation of bitshifting, negation and addition with MSW, LSW
			// This division should have a complexity of O(bitSize + (rhs.MSW - rhs.LSW)^2)
			int_limited& operator/= (int_limited rhs) {
				if (rhs == 0) throw std::domain_error("Divide by zero exception");
				int_limited dividend = *this;
				*this = 0;
				// Convert both numbers to positive, so that we can subtract the shifted rhs from the dividend
				// We also don't need to worry about the asymmetry of two's complement integer limits
				// because the result is always zero for the minimum value
				bool negative = false;
				if (dividend < 0) {
					dividend = ~dividend + 1;
					negative = !negative;
				}
				if (rhs < 0) {
					// We can afford to overwrite rhs, because it is a copy, not a reference
					rhs = ~rhs + 1;
					negative = !negative;
				}
				if (rhs > dividend) {
					// returns zero
					return *this;
				}
				int potentialLSW = dividend.LSW - rhs.MSW - 1;
				int potentialMSW = dividend.MSW - rhs.MSW + 1;
				int_limited shiftCounter = 1;
				// Now rhs <= dividend, which means rhs.MSW <= dividend.MSW
				if (rhs.MSW+1 < dividend.MSW) {
					// this will make rhs.MSW = dividend.MSW - 1
					int wordShiftCount = dividend.MSW - rhs.MSW - 1;
					shiftCounter <<= 64*wordShiftCount;
					rhs <<= 64*wordShiftCount;
				}
				// Now continue shifting rhs, so that it is one shift larger than the dividend
				while (rhs <= dividend && rhs > 0) {
					rhs <<= 1;
					shiftCounter <<= 1;
				}
				rhs >>= 1;
				shiftCounter >>= 1;

				while (shiftCounter > 0) {
					if (rhs <= dividend) {
						dividend -= rhs;
						*this |= shiftCounter;
					}
					rhs >>= 1;
					shiftCounter >>= 1;

				}
				this->updateLSW(potentialLSW);
				this->updateMSW(potentialMSW);
				if (negative) *this = ~(*this) + 1;
				// std::cout << (uint64_t)(*this >> 128) << " " << (uint64_t)(*this >> 64) << " " << (uint64_t)(*this) << std::endl;
				return *this;
			}
			int_limited operator/ (int_limited const& rhs) {
				int_limited result = *this;
				return result /= rhs;
			}

			int_limited& operator%= (int_limited rhs) {
				if (rhs == 0) throw std::domain_error("Divide by zero exception");
				bool negative = false;
				// Convert both numbers to positive, so that we can subtract the shifted rhs from (*this)
				// We also don't need to worry about the asymmetry of two's complement integer limits
				// because the result is always zero for the minimum value
				if (*this < 0) {
					// We can afford to overwrite (*this) because we will overwrite it later anyway
					*this = ~*this + 1;
					negative = !negative;
				}
				if (rhs < 0) {
					// We can afford to overwrite rhs, because it is a copy, not a reference
					rhs = ~rhs + 1;
					// Don't change (negative) here
				}
				if (rhs > *this) {
					// returns the full value
					if (negative) *this = ~(*this) + 1;
					return *this;
				}
				int potentialLSW = 0;
				int potentialMSW = rhs.MSW;
				int totalShifts = 1;
				// Now rhs <= *this, which means rhs.MSW <= this->MSW
				if (rhs.MSW+1 < this->MSW) {
					// this make rhs.MSW = this->MSW - 1
					int wordShiftCount = this->MSW - rhs.MSW - 1;
					totalShifts += 64*wordShiftCount;
					rhs <<= 64*wordShiftCount;
				}
				// Now either rhs == *this, or rhs.MSW == this->MSW - 1
				while (rhs <= *this && rhs > 0) {
					rhs <<= 1;
					totalShifts += 1;
				}
				rhs >>= 1;
				totalShifts -= 1;
				while (totalShifts > 0) {
					if (rhs <= *this) {
						*this -= rhs;
					}
					rhs >>= 1;
					totalShifts -= 1;
				}
				this->updateLSW(potentialLSW);
				this->updateMSW(potentialMSW);
				if (negative) *this = ~(*this) + 1;
				return *this;
			}
			int_limited operator% (int_limited const& rhs) {
				int_limited result = *this;
				return result %= rhs;
			}
			#pragma endregion

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
			#pragma region

			int_limited& operator^= (int_limited const& rhs) {
				for (int i = rhs.LSW; i <= rhs.MSW; i++) {
					this->words[i] ^= rhs.words[i];
				}
				this->updateLSW(std::min(this->LSW, rhs.LSW));
				this->updateMSW(std::max(this->MSW, rhs.MSW));
				return *this;
			}
			int_limited operator^ (int_limited const& rhs) {
				int_limited result = *this;
				return result ^= rhs;
			}

			int_limited& operator|= (int_limited const& rhs) {
				for (int i = rhs.LSW; i <= rhs.MSW; i++) {
					this->words[i] |= rhs.words[i];
				}
				// Although it is guaranteed to be one of the two possibilities
				// We use the function anyway incase of a redefinition
				this->updateLSW(std::min(this->LSW, rhs.LSW));
				this->updateMSW(std::max(this->MSW, rhs.MSW));
				return *this;
			}
			int_limited operator| (int_limited const& rhs) {
				int_limited result = *this;
				return result |= rhs;
			}

			int_limited& operator&= (int_limited const& rhs) {
				for (int i = std::min(this->LSW, rhs.LSW); i < this->wordCount && i <= rhs.MSW; i++) {
					this->words[i] &= rhs.words[i];
				}
				this->updateLSW(std::max(this->LSW, rhs.LSW));
				this->updateMSW(std::min(this->MSW, rhs.MSW));
				return *this;
			}
			int_limited operator& (int_limited const& rhs) {
				int_limited result = *this;
				return result &= rhs;
			}

			// Returns the bit NOT, so adding 1 gets the two's complement
			int_limited operator~ () {
				int_limited result = 0;
				for (int i = 0; i < this->wordCount; i++) {
					result.words[i] = ~this->words[i];
				}
				result.updateLSW(0);
				result.updateMSW(this->wordCount-1);
				return result;
			}

			// Classic non-arithmetic bitshift
			int_limited& operator<<= (int const& rhs) {
				int wordshift = rhs / 64;
				int bitshift = rhs % 64;
				for (int i = this->MSW; i >= this->LSW; i--) {
					this->wordShiftLeft(i, wordshift);
					this->bitShiftLeft(i, bitshift);
				}
				this->updateLSW(this->LSW + wordshift - 1);
				this->updateMSW(this->MSW + wordshift + 1);
				return *this;
			}
			// Classic non-arithmetic bitshift
			int_limited operator<< (int const& rhs) {
				int_limited result = *this;
				return result <<= rhs;
			}

			// Classic non-arithmetic bitshift
			int_limited& operator>>= (int const& rhs) {
				int wordshift = rhs / 64;
				int bitshift = rhs % 64;
				for (int i = this->LSW; i <= this->MSW; i++) {
					this->wordShiftRight(i, wordshift);
					this->bitShiftRight(i, bitshift);
				}
				this->updateLSW(this->LSW - wordshift - 1);
				this->updateMSW(this->MSW - wordshift + 1);
				return *this;
			}
			// Classic non-arithmetic bitshift
			int_limited operator>> (int const& rhs) {
				int_limited result = *this;
				return result >>= rhs;
			}
			#pragma endregion


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
			#pragma region

			bool operator== (int_limited const& rhs) {
				// If they don't have 1's in the same words, return false
				if (this->LSW != rhs.LSW || this->MSW != rhs.MSW) return false;
				// Since they have the same word range, then just check the equality of those
				for (int i = this->LSW; i <= this->MSW; i++) {
					if (this->words[i] != rhs.words[i]) return false;
				}
				return true;
			}
			bool operator!= (int_limited const& rhs) {
				return !(*this == rhs);
			}
			bool operator> (int_limited const& rhs) {
				uint64_t MSb = BIT64_ON;
				if (bitSize%64 != 0) {
					MSb >>= 64 - (bitSize%64);
				}
				// if different signs, then false if *this is negative, true if rhs is negative
				if ((this->words[this->wordCount-1] & MSb) != (rhs.words[rhs.wordCount-1] & MSb)) return this->words[wordCount-1] < rhs.words[rhs.wordCount-1];
				// Both signs are the same, so simply compare each value
				if (this->MSW != rhs.MSW) return this->MSW > rhs.MSW;
				for (int i = this->MSW; i >= this->LSW; i--) {
					if (this->words[i] != rhs.words[i]) return this->words[i] > rhs.words[i];
				}
				// if they have been equal up to here then they are either equal or rhs.LSW < this->LSW (*this < rhs)
				return false;
			}
			bool operator< (int_limited const& rhs) {
				uint64_t MSb = BIT64_ON;
				if (bitSize%64 != 0) {
					MSb >>= 64 - (bitSize%64);
				}
				// if different signs, then false if *this is negative, true if rhs is negative
				if ((this->words[this->wordCount-1] & MSb) != (rhs.words[rhs.wordCount-1] & MSb)) return this->words[wordCount-1] > rhs.words[rhs.wordCount-1];
				
				// Both signs are the same, so simply compare each value
				if (this->MSW != rhs.MSW) return this->MSW < rhs.MSW;

				for (int i = this->MSW; i >= this->LSW; i--) {
					if (this->words[i] != rhs.words[i]) return this->words[i] < rhs.words[i];
				}
				// if they have been equal up to here then they are either equal or rhs.LSW < this->LSW (*this < rhs)
				return rhs.LSW < this->LSW;
			}
			bool operator>= (int_limited const& rhs) {
				return !(*this < rhs);
			}
			bool operator<= (int_limited const& rhs) {
				return !(*this > rhs);
			}
			#pragma endregion

			/*
			SECTION: LOGICAL OPERATORS
			=============================================================
			! (NOT) DONE
			&& (AND) DONE
			|| (OR) DONE

			The only slowdown here is converting 0 to an int_limited (of any size), otherwise 
			For most cases it only compares MSW and LSW (or subsequently also word[0])
			=============================================================
			*/
			#pragma region
			// returns *this == 0
			bool operator! () {
				if (this->MSW != 0) return false;
				return this->words[0] == 0;
			}
			bool operator&& (int_limited const& rhs) {
				// if *this and rhs are non-zero
				return (!!(*this)) && (!!(rhs));
			}
			bool operator|| (int_limited const& rhs) {
				// if *this or rhs are non-zero
				return (!!(*this)) || (!!(rhs));
			}
			#pragma endregion
	};

	typedef int_limited<256> int256;
	typedef int_limited<512> int512;
	typedef int_limited<1024> int1024;
}