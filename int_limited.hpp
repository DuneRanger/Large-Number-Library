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
			importBits DONE
			exportBits DONE
			=============================================================
			*/


			// Accepts conversions from individual standard int types (doesn't change MSW and LSW)
			// To convert multiple integers of a type into an int_limited
			// It is required to declare an instance and call importBits

			// For all of these instances, LSW and MSW are by default zero, which is correct;
			int_limited() {
				static_assert(this->bitSize > 1, "Invalid int_limited size");
			}
			int_limited(uint64_t a) {
				static_assert(this->bitSize > 1, "Invalid int_limited size");
				this->words[0] = a;
			}
			int_limited(int64_t a) {
				static_assert(this->bitSize > 1, "Invalid int_limited size");
				if (a < 0) {
					for (int i = 1; i < wordCount; i++) {
						this->words[i] = UINT64_MAX;
					}
				}
				this->words[0] = a;
			}
			int_limited(int a) {
				static_assert(this->bitSize > 1, "Invalid int_limited size");
				if (a < 0) {
					for (int i = 1; i < wordCount; i++) {
						this->words[i] = UINT64_MAX;
					}
				}
				this->words[0] = (int64_t)a;
			}
			int_limited(unsigned int a) {
				static_assert(this->bitSize > 1, "Invalid int_limited size");
				this->words[0] = a;
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

			// Allows conversion *from* different size int_limited
			// However if rhs is larger, then the bits will be cut off
			int_limited& operator= (int_limited const& rhs) {
				for (int i = 0; i < wordCount && i <= rhs.LWS; i++) {
					this->words[i] = rhs.words[i];
				}
				return *this;
			}

			// For simplicity's sake this function only accepts
			// a vector of unsigned 64 bit integers from the standard library
			void importBits(std::vector<uint64_t> newWords) {
				for (int i = 0; i < wordCount && i < newWords.size(); i++) {
					this->words[i] = newWords[i];
				}
				this->updateLSW(0);
				this->updateMSW(newWords.size()-1);
			}

			// For simplicity's sake this function only returns
			// a vector of unsigned 64 bit integers from the standard library
			// Currently returns *all* words, even those higher than the Most Significant Word
			std::vector<uint64_t> exportBits() {
				return this->words;
			}

			/*
			SECTION: HELPER FUNCTIONS
			=============================================================
			Update LSW DONE
			Update MSW DONE
			Word Shift Left DONE
			Word Shift Right DONE
			Bit Shift Left DONE
			Bit Shift Right DONE
			=============================================================
			*/
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
				upperBound = std::min(this->wordCount, upperBound);
				// Find the highest non-zero word
				while (this->words[upperBound] == 0) {
					upperBound--;
				}
				// If the number is zero
				if (upperBound < 0) upperBound = 0;
				this->MSW = upperBound;
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

			/*
			SECTION: PRINTING
			=============================================================
			toString DONE
			<< (insertion to stream) DONE
			>> (extraction from stream)
			=============================================================
			*/
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
				int maxWordCount = this->bitSize/(binWordSize - 1) + 1;

				// Convert this into a vector of uin64_t chunks (yes, it is a bit wasteful for low bases)
				std::vector<uint64_t> baseWords(maxWordCount, 0);
				int_limited<this->bitSize> num = *this;
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
			* (multiplication)
			/ (division)
			% (modulus)
			respective compound operators (+=, -=, *=, /=, %=)
			=============================================================
			*/
			
			int_limited& operator+=(int_limited const& rhs) {
				bool carry = false;
				// Cycle from the lowest LSW to max(this.wordCount, rhs.MSW)
				// to end the earliest we can
				for (int i = std::min(LSW, rhs.LSW); i < this->wordCount && i <= rhs.MSW; i++) {
					char flag1 = (this->words[i] >= BIT64_ON) + (rhs->words[i] >= BIT64_ON);
					this->words[i] += rhs->words[i];
					this->words[i] += carry;
					bool flag2 = this->words[i] < BIT64_ON;
					// carry = (either both had BIT64_ON), or (only one had it on and the sum didn't)
					carry = (flag1 + flag2 > 1);
				}
				uint64_t rhsExtension = 0;
				if (rhs < 0) rhsExtension = UINT64_MAX;
				if (this->MSW - 1 > rhs.MSW) {
					// If the carry can still affect anything (theoretically up to (exclusive)this->wordCount, then continue addition
					// Also must continue if rhs requires a one extension
					if (carry || rhsExtension) {
						// Marks the last word that we will still operate on
						int lastAffectedWord = this->wordCount;
						for (int i = rhs.MSW; i < lastAffectedWord; i++) {
							char flag1 = (this->words[i] >= BIT64_ON) && (rhsExtension != 0);
							this->words[i] += rhsExtension;
							this->words[i] += carry;
							bool flag2 = this->words[i] < BIT64_ON;
							carry = (flag1 && flag2);
							// If we can finish early, the break
							if (!carry && rhsExtension == 0) break;
						}
					}
				}

				this->updateLSW(std::min(this->LSW, rhs.LSW));
				this->updateMSW(std::max(this->MSW, rhs.MSW) + 1); // +1 for potential carry
				return *this;
			}
			int_limited operator+(int_limited const& rhs) {
				int_limited result = *this;
				return result += rhs;
			}

			int_limited& operator-=(int_limited rhs) {
				return *this += (~rhs + 1);
			}
			int_limited operator-(int_limited rhs) {
				int_limited result = *this;
				return result += (~rhs + 1);
			}
			// negates value
			int_limited operator-() {
				int_limited result = *this;
				return (~result + 1);
			}


			/*
			SECTION: BITWISE OPERATORS
			=============================================================
			^ (XOR) DONE
			| (OR) DONE
			& (AND) DONE
			~ (complement) DONE
			<< (shift left)
			>> (shift right)
			respective compound operators (^=, |=, &=, <<=, >>=)
			=============================================================
			*/

			int_limited& operator^= (int_limited const& rhs) {
				for (int i = std::min(this->LSW, this.LSW); i < this->wordCount && i <= rhs.MSW; i++) {
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
				for (int i = std::min(this->LSW, this.LSW); i < this->wordCount && i <= rhs.MSW; i++) {
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
				for (int i = std::min(this->LSW, this.LSW); i < this->wordCount && i <= rhs.MSW; i++) {
					this->words[i] &= rhs.words[i];
				}
				this->updateLSW(std::min(this->LSW, rhs.LSW));
				this->updateMSW(std::max(this->MSW, rhs.MSW));
				return *this;
			}
			int_limited operator& (int_limited const& rhs) {
				int_limited result = *this;
				return result &= rhs;
			}

			// Returns the bit NOT, so adding 1 gets the two's complement
			int_limited operator~ () {
				// If this doesn't work, then just do result = *this and double operation time
				int_limited<this->bitSize> result = 0;
				for (int i = this->LSW; i <= this->MSW; i++) {
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

			// Compares values regardless of bitSize (purely by value)
			bool operator== (int_limited const& rhs) {
				// If they don't have 1's in the same words, return false
				// This also guarantees that going out of index won't happen during later comparison
				if (this->LSW != rhs.LSW || this->MSW != rhs.MSW) return false;
				for (int i = this->LSW; i <= this->MSW; i++) {
					if (this->words[i] != rhs.words[i]) return false;
				}
				return true;
			}
			// Compares values regardless of bitSize (purely by value)
			bool operator!= (int_limited const& rhs) {
				return !(*this == rhs);
			}
			// Compares values regardless of bitSize (purely by value)
			bool operator> (int_limited const& rhs) {
				// if different signs - false if *this is negative, true if rhs is negative
				bool isNegative = this->words[this->wordCount-1] & BIT64_ON;
				if (isNegative != (rhs.words[rhs.wordCount-1] & BIT64_ON)) return this->words[wordCount-1] < rhs.words[rhs.wordCount-1];
				
				// Both signs are the same, so both numbers are positive here
				if (!isNegative) {
					// if one of them has a more significant bit, return based on that
					if (this->MSW != rhs.MSW) return this->MSW > rhs.MSW;
					// If they have the same MSW, then start comparing from there
					for (int i = this->MSW; i > -1; i--) {
						if (this->words[i] != rhs.words[i]) return this->words[i] > rhs.words[i];
					}
				} else { // Here they are both negative
					int leftMSW = this->MSW;
					int rightMSW = rhs.MSW;
					for (; leftMSW > rightMSW; leftMSW--) {
						// If this number is "longer" and isn't just a two's complement extension of rhs
						// then it is smaller (due to containing a zero)
						if (this->words[leftMSW] != UINT64_MAX) return false;
					}
					for (; rightMSW > leftMSW; rightMSW--) {
						// The opposite situation of above
						if (rhs.words[rightMSW] != UINT64_MAX) return true;
					}
					// Here both MSW are equal
					for (; leftMSW > -1; leftMSW--) {
						if (this->words[leftMSW] != rhs.words[leftMSW]) return this->words[leftMSW] > rhs.words[leftMSW];
					}
				}
				// Occurs when they are equal
				return false;
			}
			// Compares values regardless of bitSize (purely by value)
			bool operator< (int_limited const& rhs) {
				// if different signs, then false if *this is negative, true if rhs is negative
				bool isNegative = this->words[this->wordCount-1] & BIT64_ON;
				if (isNegative != (rhs.words[rhs.wordCount-1] & BIT64_ON)) return this->words[wordCount-1] > rhs.words[rhs.wordCount-1];
				
				// Both signs are the same, so both numbers are positive here
				if (!isNegative) {
					// if one of them has a more significant bit, return based on that
					if (this->MSW != rhs.MSW) return this->MSW < rhs.MSW;
					// If they have the same MSW, then start comparing from there
					for (int i = this->MSW; i > -1; i--) {
						if (this->words[i] != rhs.words[i]) return this->words[i] < rhs.words[i];
					}
				} else { // Here they are both negative
					int leftMSW = this->MSW;
					int rightMSW = rhs.MSW;
					for (; leftMSW > rightMSW; leftMSW--) {
						// If this number is "longer" and isn't just a two's complement extension of rhs
						// then it is smaller (due to containing a zero)
						if (this->words[leftMSW] != UINT64_MAX) return true;
					}
					for (; rightMSW > leftMSW; rightMSW--) {
						// The opposite situation of above
						if (rhs.words[rightMSW] != UINT64_MAX) return false;
					}
					// Here both MSW are equal
					for (; leftMSW > -1; leftMSW--) {
						if (this->words[leftMSW] != rhs.words[leftMSW]) return this->words[leftMSW] < rhs.words[leftMSW];
					}
				}
				// Occurs when they are equal
				return false;
			}
			// Compares values regardless of bitSize (purely by value)
			bool operator>= (int_limited const& rhs) {
				return !(*this < rhs);
			}
			// Compares values regardless of bitSize (purely by value)
			bool operator<= (int_limited const& rhs) {
				return !(*this > rhs);
			}

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
			// Arguments do not need to have equal bitSize
			bool operator! () {
				return *this == 0;
			}
			// Arguments do not need to have equal bitSize
			bool operator&& (int_limited const& rhs) {
				return (*this != 0) && (rhs != 0);
			}
			// Arguments do not need to have equal bitSize
			bool operator|| (int_limited const& rhs) {
				return (*this != 0) || (rhs != 0);
			}
	};
}