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
					for (int i = 1; i < wordCount; i++) {
						this->words[i] = UINT64_MAX;
					}
				}
				this->words[0] = a;
				this->truncateExtraBits();
			}
			int_limited(int a) {
				static_assert(bitSize > 1, "Invalid int_limited size");
				if (a < 0) {
					for (int i = 1; i < wordCount; i++) {
						this->words[i] = UINT64_MAX;
					}
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
				for (int i = rhs.LSW; i <= rhs.MSW; i++) {
					this->words[i] = rhs.words[i];
				}
				this->updateLSW(rhs.LSW);
				this->updateMSW(rhs.MSW);
				return *this;
			}

			// For simplicity's sake this function only accepts
			// a vector of unsigned 64 bit integers from the standard library
			void importBits(std::vector<uint64_t>& newWords) {
				for (int i = 0; i < this->wordCount && i < newWords.size(); i++) {
					this->words[i] = newWords[i];
				}
				this->updateLSW(0);
				this->updateMSW(newWords.size()-1);
				return;
			}

			// Starts importing from startIndex (inclusive) to endIndex (exclusive)
			// Import into the destinations words, starting from index 0
			void importBits(std::vector<uint64_t>& newWords, int startIndex, int endIndex) {
				int maxWord = std::min(this->wordCount, (int)newWords.size(), endIndex);
				for (int i = startIndex; i < maxWord; i++) {
					this->words[i] = newWords[i];
				}
				this->updateLSW(0);
				this->updateMSW(newWords.size()-1);
				return;
			}

			// The first iterator is taken as the Least Significant Word (+ wordOffset)
			// The second iterator is non-inclusive in the import
			void importBits(std::vector<uint64_t>::iterator beginWords, std::vector<uint64_t>::iterator endWords, int wordOffset = 0) {
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
			=============================================================
			*/
			#pragma region

			void truncateExtraBits() {
				int bitsInMSW = bitSize % 64;
				if (bitsInMSW == 0) return;
				this->words[this->wordCount-1] &= UINT64_MAX >> (64 - bitsInMSW);
				return;
			}

			void updateLSW(int lowerBound) {
				lowerBound = std::max(0, lowerBound);
				// Find the highest non-zero word
				while (this->words[lowerBound] == 0) {
					lowerBound++;
				}
				// If the number is zero
				if (lowerBound > this->wordCount) lowerBound = 0;
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
				if (wordIndex + shift < this->wordCount) {
					this->words[wordIndex + shift] = this->words[wordIndex];
				}
				return;
			}

			// Shift a whole word right by an amount of words
			void wordShiftRight(int wordIndex, int shift) {
				if (wordIndex - shift > -1) {
					this->words[wordIndex - shift] = this->words[wordIndex];
				}
				return;
			}

			// Shifts a whole word left by an amount of bits
			void bitShiftLeft(int wordIndex, int shift) {
				if (shift == 0) return;
				if (wordIndex + 1 < this->wordCount) {
					this->words[wordIndex + 1] = this->words[wordIndex] >> (64 - shift);
				}
				this->words[wordIndex] <<= shift;
				return;
			}

			// Shifts a whole word right by an amount of bits
			void bitShiftRight(int wordIndex, int shift) {
				if (shift == 0) return;
				if (wordIndex - 1 > -1) {
					this->words[wordIndex - 1] = this->words[wordIndex] << (64 - shift);
				}
				this->words[wordIndex] >>= shift;
				return;
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
				return "customBigInt::int_limited";
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

				// Convert this into a vector of uin64_t chunks (yes, it is a bit wasteful for low bases)
				std::vector<uint64_t> baseWords(maxWordCount, 0);
				int_limited num = *this;
				bool sign = num < 0;
				int index = maxWordCount-1;
				if (sign) num = ~num+1;
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
				// if (this->MSW - 1 > rhs.MSW) {
				// 	// If the carry can still affect anything (theoretically up to (exclusive)this->wordCount, then continue addition
				// 	// Also must continue if rhs requires a one extension
				// 	if (carry || rhsExtension) {
				// 		// Marks the last word that we will still operate on
				// 		int lastAffectedWord = this->wordCount;
				// 		for (int i = rhs.MSW; i < lastAffectedWord; i++) {
				// 			char flag1 = (this->words[i] >= BIT64_ON) && (rhsExtension != 0);
				// 			this->words[i] += rhsExtension;
				// 			this->words[i] += carry;
				// 			bool flag2 = this->words[i] < BIT64_ON;
				// 			carry = (flag1 && flag2);
				// 			// If we can finish early, the break
				// 			if (!carry && rhsExtension == 0) break;
				// 		}
				// 	}
				// }

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

			// Multiplication done by Karatsuba's algorithm
			int_limited& operator*= (int_limited rhs) {
				// By converting both numbers to positive ones, we can multiply small negative numbers much faster
				bool negative = false;
				if (*this < 0) {
					// We can afford to overwrite (*this) because we will overwrite it later anyway
					*this = ~(*this) + 1;
					negative = !negative;
				}
				if (rhs < 0) {
					// We can afford to overwrite rhs, because it is a copy, not a reference
					rhs = ~rhs + 1;
					negative = !negative;
				}
				// The max word count only counts the amount of words used for values
				// So that we can uncover 64 bit values in instances with a larger bitSize
				int maxWordCount = std::max(this->MSW, rhs.MSW);
				// If both values are only 64 bit/1 word
				if (maxWordCount == 0) {
					// If the result fits within 64 bits
					if (*this <= UINT32_MAX && rhs <= UINT32_MAX) {
						this->words[0] *= rhs.words[0];
						if (negative) *this = ~(*this) + 1;
						return *this;
					}
					// Otherwise just manually divide the words and overwrite *this
					uint64_t lhsWord = this->words[0];
					uint64_t rhsWord = rhs.words[0];
					*this = (lhsWord >> 32) * (rhsWord >> 32);
					*this <<= 32;
					*this += (lhsWord >> 32) * (rhsWord & UINT32_MAX);
					*this += (lhsWord & UINT32_MAX) * (rhsWord >> 32);
					*this <<= 32;
					*this += (lhsWord & UINT32_MAX) * (rhsWord & UINT32_MAX);
					if (negative) *this = ~(*this) + 1;
					this->updateLSW(0);
					this->updateMSW(1);
					return *this;
				}
				// The plan here is to skip the most significant half of the words, because they will overflow in the result anyway
				// But unfortunately, since bitSize is part of a template, then all recursive calls will have this many int_limited<bitSize> variables
				// So the memory usage will be rather high (in terms of constants)
				int splitWordIndex = maxWordCount / 2;

				int_limited low1 = 0;
				low1.importBits(this->words, 0, splitWordIndex);
				int_limited high1 = 0;
				high1.importBits(this->words, splitWordIndex, this->wordCount);

				int_limited low2 = 0;
				low2.importBits(rhs.words, 0, splitWordIndex);
				int_limited high2 = 0;
				high2.importBits(rhs.words, splitWordIndex, rhs.wordCount);

				int_limited z0 = low1 * low2;
				int_limited z1 = (high1 + low1) * (high2 + low2);
				int_limited z2 = high1 * high2;

				*this = z2;
				*this <<= splitWordIndex*64;
				*this += (z1 - z2 - z0);
				*this <<= splitWordIndex*64;
				*this += z0;
				this->updateLSW(this->LSW);
				this->updateMSW(this->wordCount);
				if (negative) *this = ~(*this);
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
				// By converting both numbers to positive ones, we can divide small negative numbers much faster
				bool negative = false;
				if (dividend < 0) {
					// We can afford to overwrite (dividend) because we will overwrite it later anyway
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
				int_limited shiftCounter = 1;
				// Now rhs <= dividend, which means rhs.MSW <= dividend.MSW
				if (rhs.MSW < dividend.MSW) {
					// this make rhs.MSW = dividend.MSW - 1, if it was originally smaller
					rhs <<= 64*(dividend.MSW - rhs.MSW);
					shiftCounter <<= 64;
				}
				// Now either rhs == dividend, or rhs.MSW == dividend.MSW - 1
				while (rhs <= dividend) {
					rhs <<= 1;
					shiftCounter <<= 1;
				}
				while (shiftCounter > 0) {
					if (rhs <= dividend) {
						dividend -= rhs;
						*this |= shiftCounter;
					}
					rhs >>= 1;
					shiftCounter >>= 1;
				}
				this->updateLSW(0);
				this->updateMSW(this->MSW);
				if (negative) *this = ~(*this) + 1;
				return *this;
			}
			int_limited operator/ (int_limited const& rhs) {
				int_limited result = *this;
				return result /= rhs;
			}

			int_limited& operator%= (int_limited rhs) {
				if (rhs == 0) throw std::domain_error("Divide by zero exception");
				// By converting both numbers to positive ones, we can divide by small negative numbers much faster
				bool negative = false;
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
				int totalShifts = 1;
				// Now rhs <= *this, which means rhs.MSW <= this->MSW
				if (rhs.MSW < this->MSW) {
					// this make rhs.MSW = this->MSW - 1, if it was originally smaller
					rhs <<= 64*(this->MSW - rhs.MSW);
					totalShifts += 64;
				}
				// Now either rhs == *this, or rhs.MSW == this->MSW - 1
				while (rhs <= *this) {
					rhs <<= 1;
					totalShifts += 1;
				}
				while (totalShifts > 0) {
					if (rhs <= *this) {
						*this -= rhs;
					}
					rhs >>= 1;
					totalShifts -= 1;
				}
				this->updateLSW(0);
				this->updateMSW(this->MSW);
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
				// If this doesn't work, then just do result = *this and double operation time
				int_limited result = 0;
				for (int i = 0; i < this->wordCount; i++) {
					result.words[i] = ~this->words[i];
				}
				this->updateLSW(0);
				this->updateMSW(this->wordCount-1);
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
				this->updateLSW(this->LSW + wordshift);
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
				this->updateMSW(this->MSW - wordshift);
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
				// if different signs - false if *this is negative, true if rhs is negative
				bool isNegative = this->words[this->wordCount-1] & BIT64_ON;
				if (isNegative != (rhs.words[rhs.wordCount-1] & BIT64_ON)) return this->words[wordCount-1] < rhs.words[rhs.wordCount-1];
				
				// Both signs are the same, so simply compare each value
				if (this->MSW != rhs.MSW) return this->MSW > rhs.MSW;

				for (int i = this->MSW; i >= this->LSW; i--) {
					if (this->words[i] != rhs.words[i]) return this->words[i] > rhs.words[i];
				}
				// if they have been equal up to here then they are either equal or rhs.LSW < this->LSW (*this < rhs)
				return false;
			}
			bool operator< (int_limited const& rhs) {
				// if different signs, then false if *this is negative, true if rhs is negative
				bool isNegative = this->words[this->wordCount-1] & BIT64_ON;
				if (isNegative != (rhs.words[rhs.wordCount-1] & BIT64_ON)) return this->words[wordCount-1] > rhs.words[rhs.wordCount-1];
				
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