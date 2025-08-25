#pragma once
#include "constants.hpp"
#include <cstdint>
#include <iostream>
#include <string>
#include <vector>
#include <array>


namespace largeNumberLibrary {
	// Two's complement
	// Constructs an array of 32 bit unsigned integers, so that the specified bit size fits
	// If the bitSize isn't a multiple of 32, operations will still be processed for all 32 bits of the most significant word
	// However overflow will still occur if the value were to surpass the bitSize
	// Comparisons will also ignore any extra bits above the bitSize
	// No further optimizations are made on the most significant word (even if the instance only has 1 word)
	// All operations occur on class instances with equal bitSize
	// In case of different sizes, both values are set to the size of the left hand side
	template <int bitSize>
	class int_limited {
		private:
			static const int wordCount = bitSize/32 + (bitSize%32 > 0);

			// LSb first
			// The most significant word is the last one
			std::array<uint32_t, wordCount> words = {};

			// Most and Least Significant Word containing a non-zero bit
			// Used for a optimization for arithmetic operations
			// Doesn't really help for small negative numbers - they require negation
			int MSW = 0;
			int LSW = 0;

			// required to simplify division (extra precision)
			template <int exPr> friend class int_limited;

			/*
			SECTION: HELPER FUNCTIONS
			=============================================================
			Concat 32 bits to 64 DONE
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
			#pragma region Helper

			static uint64_t concatTo64Bit(uint32_t A, uint32_t B) {
				return (uint64_t(A) << 32) | B;
			}

			void truncateExtraBits() {
				int bitsInMSW = bitSize % 32;
				if (bitsInMSW == 0) return;
				this->words[this->wordCount-1] &= UINT32_MAX >> (32 - bitsInMSW);
				if (this->words[this->wordCount-1] == 0 && this->wordCount > 1) this->updateMSW(this->MSW);
				return;
			}

			void updateLSW(int lowerBound) {
				lowerBound = std::max(0, lowerBound);
				lowerBound = std::min(lowerBound, this->wordCount - 1);
				// Find the highest non-zero word
				while (lowerBound < this->wordCount && this->words[lowerBound] == 0) {
					lowerBound++;
				}
				// if all values are zero
				if (lowerBound == this->wordCount) lowerBound = 0;
				this->LSW = lowerBound;
				return;
			}

			void updateMSW(int upperBound) {
				upperBound = std::min(this->wordCount - 1, upperBound);
				upperBound = std::max(upperBound, 0);
				// Find the highest non-zero word
				while (upperBound > -1 && this->words[upperBound] == 0) {
					upperBound--;
				}
				// occurs when the value is zero
				// there is no other way to check other then going through the whole value
				if (upperBound == -1) upperBound = 0;

				this->MSW = upperBound;
				if (this->MSW == this->wordCount - 1) {
					this->truncateExtraBits();
				}
				return;
			}
			
			// Shift a single word left by an amount of words
			void wordShiftLeft(int wordIndex, int shift) {
				if (shift == 0) return;
				if (wordIndex + shift < this->wordCount) {
					this->words[wordIndex + shift] = this->words[wordIndex];
				}
				this->words[wordIndex] = 0;
				return;
			}

			// Shift a single word right by an amount of words
			void wordShiftRight(int wordIndex, int shift) {
				if (shift == 0) return;
				if (wordIndex - shift > -1) {
					this->words[wordIndex - shift] = this->words[wordIndex];
				}
				this->words[wordIndex] = 0;
				return;
			}

			// Shifts a single word left by an amount of bits
			// Expects the shift to be less than or equal to 32 bits
			void bitShiftLeft(int wordIndex, int shift) {
				if (shift == 0) return;
				if (wordIndex + 1 < this->wordCount) {
					this->words[wordIndex + 1] |= this->words[wordIndex] >> (32 - shift);
				}
				if (wordIndex < this->wordCount) this->words[wordIndex] <<= shift;
				return;
			}

			// Shifts a single word right by an amount of bits
			// Expects the shift to be less than or equal to 32 bits
			void bitShiftRight(int wordIndex, int shift) {
				if (shift == 0) return;
				if (wordIndex - 1 > -1) {
					this->words[wordIndex - 1] |= this->words[wordIndex] << (32 - shift);
				}
				if (wordIndex > -1) this->words[wordIndex] >>= shift;
				return;
			}

			int_limited& basicMult(int_limited const& A, int_limited const& B) {
				*this = 0;
				if (!A || !B) return *this;
				for (int b_i = B.LSW; b_i <= B.MSW; b_i++) {
					uint32_t carry = 0;
					bool secondCarry = false;
					for (int a_i = A.LSW; a_i <= A.MSW; a_i++) {
						// If the result is outside of precision, continue
						if (a_i + b_i >= this->wordCount) continue;

						// If some of the lower 32 bits of the result fit into the integer
						// Then multiply with only 32 bit precision
						if (a_i + b_i + 1 == this->wordCount) {
							this->words[a_i + b_i] +=  A.words[a_i] * B.words[b_i] + carry + secondCarry;
							// No need to set carry, because it will be out of precision next iteration
							continue;
						}
						

						// product cannot overflow, unless carry = (UINT32_MAX << 1) + 1
						uint64_t product = carry;
						product += secondCarry;
						uint64_t multiplicand = A.words[a_i];
						uint32_t multiplier = B.words[b_i];
						product += multiplicand * multiplier;
						char flag1 = (this->words[a_i + b_i] >= BIT32_ON) + ((uint32_t)product >= BIT32_ON);
						this->words[a_i + b_i] += (uint32_t)product;
						bool flag2 = this->words[a_i + b_i] < BIT32_ON;
						// The maximum value of (product >> 32) is UINT32_MAX, which means that
						// by itself, it can fit in the carry, but (flag1 && flag2) doesn't have to
						// That is why we separate them (e.g. -1*-1 with Karatsuba's algorithm - 512+ bits)
						carry = (uint32_t)(product >> 32);
						secondCarry = (flag1 + flag2) > 1;
					}

					if (b_i + A.MSW + 1 < this->wordCount) {
						// if (carry + secondCarry) overflows and will fit into precision, then overflow
						if (secondCarry && (carry == UINT32_MAX) && ((b_i + A.MSW + 2) < this->wordCount)) this->words[b_i + A.MSW + 2] = 1;
						// Otherwise simply add regardless of overflow/precision
						else this->words[b_i + A.MSW + 1] = carry + secondCarry;
					}

					// No need to worry about checking for a new carry, because [b_i + A.MSW + 1] is guaranteed to have been empty at this point
				}
				this->updateLSW(A.LSW); // B in non-zero, so it can only grow
				this->updateMSW(A.MSW + B.MSW + 1);
				return *this;
			}
			#pragma endregion Helper

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
			importBits (vector) DONE
			importBits (vector, startIndex, endIndex, wordOffset) DONE
			importBits (iterator to iterator) DONE
			exportBits DONE
			MAX value DONE
			MIN value DONE
			=============================================================
			*/
			#pragma region Construction

			// Accepts conversions from individual standard int types (doesn't change MSW and LSW)
			// To convert multiple integers of a type into an int_limited
			// It is required to declare an instance and call importBits

			// For all of these instances, LSW and MSW are by default zero, which is correct;
			int_limited() {
				static_assert(bitSize > 1, "Invalid int_limited size");
			}
			int_limited(uint64_t a) {
				static_assert(bitSize > 1, "Invalid int_limited size");
				this->words[0] = uint32_t(a);
				if (wordCount > 1) this->words[1] = a >> 32;
				this->updateMSW(1);
				this->truncateExtraBits();
			}
			int_limited(int64_t a) {
				static_assert(bitSize > 1, "Invalid int_limited size");
				if (a < 0) {
					// iterates from i=0 to stop warning from compiling with g++ -O2 or higher
					for (int i = 0; i < this->wordCount; i++) {
						this->words[i] = UINT32_MAX;
					}
				}
				this->words[0] = a & UINT32_MAX;
				if (wordCount > 1) this->words[1] = a >> 32;
				if (a > 0) this->updateMSW(1);
				else this->updateMSW(this->wordCount - 1);
				this->truncateExtraBits();
			}
			int_limited(int a) {
				static_assert(bitSize > 1, "Invalid int_limited size");
				if (a < 0) {
					// iterates from i=0 to stop warning from compiling with g++ -O2 or higher
					for (int i = 0; i < this->wordCount; i++) {
						this->words[i] = UINT32_MAX;
					}
					this->updateMSW(this->wordCount - 1);
				}
				this->words[0] = a;
				this->truncateExtraBits();
			}
			int_limited(unsigned int a) {
				static_assert(bitSize > 1, "Invalid int_limited size");
				this->words[0] = a;
				this->truncateExtraBits();
			}
			int_limited(const char* s) {
				bool negative = false;
				if (*s != '\0' && *s == '-') {
					negative = true;
					s++;
				}
				while (*s != '\0') {
					const char c = *s;
					if (c < '0' || c > '9') throw std::domain_error("String to int_limited conversion exception");
					// multiply by ten
					*this = ((*this << 2) + *this) << 1;
					*this += (c-'0');
					s++;
				}
				if (negative) *this = ~*this + 1;
			}
			int_limited(const std::string s) {
				bool negative = false;
				int i = 0;
				if (s.size() > 0 && s[0] == '-') {
					negative = true;
					i++;
				}
				for (; i < s.size(); i++) {
					const char c = s[i];
					if (c < '0' || c > '9') throw std::domain_error("String to int_limited conversion exception");
					// multiply by ten
					*this = ((*this << 2) + *this) << 1;
					*this += (c-'0');
				}
				if (negative) *this = ~*this + 1;
			}
			template<int rhs_size>
			int_limited(int_limited<rhs_size> const& rhs) {
				if (rhs >= 0) this->importBits(rhs.exportBits());
				else {
					this->importBits((~rhs + 1).exportBits());
					*this = ~(*this) + 1;
				}
			}

			// All explicit conversions simply returns the bits for the given bit amount
			// For example the minimum value (in two's complement) converted to a int64_t will simply return 0
			explicit operator uint64_t() const {
				if (wordCount < 2) return this->words[0];
				return (uint64_t(this->words[1]) << 32) | this->words[0];
			}
			// Simply returns LSB to allow for easier bit manipulation
			explicit operator int64_t() const {
				if (wordCount < 2) return int64_t(this->words[0]);
				return (int64_t(this->words[1]) << 32) | this->words[0];
			}
			explicit operator int() const {
				return (int)this->words[0];
			}
			explicit operator unsigned int() const {
				return this->words[0];
			}
			explicit operator char() const {
				return (char)this->words[0];
			}

			int_limited& operator= (int_limited const& rhs) {
				// Will rewrite all bits, because this->wordCount == rhs.wordCount
				for (int i = 0; i < rhs.wordCount; i++) {
					this->words[i] = rhs.words[i];
				}
				this->updateLSW(rhs.LSW);
				this->updateMSW(rhs.MSW);
				return *this;
			}

			// For simplicity's sake this function only accepts
			// a vector of unsigned 32 bit integers from the standard library
			void importBits(std::vector<uint32_t> const& newWords) {
				for (int i = 0; i < this->wordCount && i < newWords.size(); i++) {
					this->words[i] = newWords[i];
				}
				for (int i = newWords.size(); i < this->wordCount; i++) {
					this->words[i] = 0;
				}
				this->updateLSW(0);
				this->updateMSW(newWords.size()-1);
				return;
			}

			// Starts importing from newWords[startIndex] (inclusive) to newWords[endIndex - 1]
			// Import into the destinations words, starting from wordOffset
			void importBits(std::vector<uint32_t> const& newWords, int startIndex, int endIndex, int wordOffset = 0) {
				if (startIndex < 0 || endIndex < 0 || wordCount < 0) {
					throw std::range_error("Invalid argument for importBits");
				}
				int maxWord = std::min((int)newWords.size(), endIndex);
				maxWord = std::min(maxWord, startIndex + (this->wordCount - wordOffset));

				for (int i = 0; i < wordOffset; i++) {
					this->words[i] = 0;
				}
				for (int i = startIndex; i < maxWord; i++) {
					this->words[wordOffset] = newWords[i];
					wordOffset++;
				}
				for (int i = maxWord; i < this->wordCount; i++) {
					this->words[i] = 0;
				}
				this->updateLSW(wordOffset);
				this->updateMSW(endIndex - startIndex + wordOffset + 1);
				return;
			}

			// The first iterator is taken as the Least Significant Word (+ wordOffset)
			// The second iterator is non-inclusive in the import
			void importBits(std::vector<uint32_t>::iterator beginWords, std::vector<uint32_t>::iterator endWords, int wordOffset = 0) {
				if (wordCount < 0) {
					throw std::range_error("Invalid wordOffset for importBits");
				}
				for (int i = 0; i < wordOffset; i++) {
					this->words[i] = 0;
				}
				int index = wordOffset;
				while (beginWords != endWords) {
					this->words[index] = *beginWords;
					beginWords++;
					index++;
				}
				for (int i = index; i < this->wordCount; i++) {
					this->words[i] = 0;
				}
				this->updateLSW(wordOffset);
				this->updateMSW(index - 1);
				return;
			}

			// For simplicity's sake this function only returns
			// a vector of unsigned 32 bit integers from the standard library
			// Currently returns *all* words, even those higher than the Most Significant (used) Word
			std::vector<uint32_t> exportBits() const {
				std::vector<uint32_t> converted_words(wordCount, 0);
				for (int i = this->LSW; i <= this->MSW; i++) {
					converted_words[i] = this->words[i];
				}
				return converted_words;
			}

			static int_limited MAX_VALUE() {
				return int_limited(-1) >> 1;
			}

			static int_limited MIN_VALUE() {
				return int_limited(1) << (bitSize-1);
			}
			#pragma endregion Construction


			/*
			SECTION: PRINTING
			=============================================================
			className DONE
			toString DONE
			<< (insertion to stream) DONE
			=============================================================
			*/
			#pragma region Printing

			static std::string className() {
				return "largeNumberLibrary::int_limited<" + std::to_string(bitSize) + ">";
			}

			// Returns a string of the current value converted to the desired base
			// '-' is appended to the start, if the number is negative, irregardless of the base
			// Base is limited to a single unsigned 32 bit integer
			std::string toString(uint32_t base = 10) const {
				if (base == 0) throw std::out_of_range("Unable to convert value to base 0");
				// special case, because it requires special attention
				if (*this == 0) return "0";
				// Approximate of the largest number of possible words in the chosen base
				int binWordSize = 0;
				uint32_t base_copy = base;
				while (base_copy != 0) {
					base_copy >>= 1;
					binWordSize++;
				}
				// -1 to binWordSize to account for unfilled bits
				if (binWordSize > 1) binWordSize--;
				// +1 at the end to act as a ceil() for special cases
				int maxWordCount = bitSize/(binWordSize) + 1;

				// Convert this into a vector of uin32_t chunks (a bit wasteful for low bases)
				std::vector<uint32_t> baseWords(maxWordCount, 0);
				int_limited num = *this;
				bool sign = num < 0;
				bool add = false;
				if (sign) {
					add = (num == int_limited::MIN_VALUE());
					if (add) num += 1;
					num = ~num+1;
				}
				// baseWords will be in MSb first, for ease of conversion to string
				int index = maxWordCount-1;
				do {
					baseWords[index] = (uint32_t)(num%base);
					num /= base;
					index--;
				} while (num != 0 && index >= 0);
				// Convert the word-size chunks into the string
				std::string output = "";
				if (sign) output += '-';
				for (uint32_t word : baseWords) {
					// skip leading zeroes
					if (output.size() <= sign && word == 0) continue;
					if (base <= 10) {
						output += '0' + (char)word;
					} else if (base < 37) { // the whole alphabet
						char charWord = '0' + (char)word;
						if (word > 9) charWord = 'A' + (char)word - 10;
						output += charWord;
					} else {
						output += std::to_string(word) + "_"; // + word divider for clarity
					}
				}
				if (add) output[output.size()-1] = char(output[output.size()-1] + 1);
				return output;
			}

			// Note: This overload doesn't take a reference, because it would throw an error when printing a complex expression
			// For example (a * -1)
			// It also doesn't consider num as a const, because methods can't be called on consts (at least from my understanding of the error)
			friend std::ostream& operator<<(std::ostream& os, int_limited const& num) {
				os << num.toString();
				return os;
			}
			#pragma endregion Printing


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
			#pragma region Arithmetic
			
			int_limited& operator+= (int_limited const& rhs) {
				bool carry = false;
				// Cycle from the lowest word in rhs with a non-zero value
				for (int i = rhs.LSW; i <= rhs.MSW; i++) {
					uint64_t sum = uint64_t(this->words[i]) + rhs.words[i] + carry;
					this->words[i] = sum & UINT32_MAX;
					carry = sum > UINT32_MAX;
				}
				// Propagate the carry to other words in *this
				for (int i = rhs.MSW + 1; i < this->wordCount; i++) {
					this->words[i] += carry;
					
					if (this->words[i] != 0) break;
				}

				this->updateLSW(std::min(this->LSW, rhs.LSW));
				this->updateMSW(std::max(this->MSW, rhs.MSW) + 1); // +1 for potential carry
				return *this;
			}
			int_limited operator+ (int_limited const& rhs) const {
				int_limited result = *this;
				return result += rhs;
			}

			int_limited& operator-= (int_limited const& rhs) {
				return *this += (~rhs + 1);
			}
			int_limited operator- (int_limited const& rhs) const {
				int_limited result = *this;
				return result += (~rhs + 1);
			}
			// negates value
			int_limited operator- () const {
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
				if (B.MSW <= 16) {
					return this->basicMult(A, B);
				}
				int maxWordAmount = A.MSW;
				int splitWordIndex  = maxWordAmount / 2 + 1;
				int_limited lowA;
				lowA.importBits(A.exportBits(), 0, splitWordIndex);
				int_limited highA;
				highA.importBits(A.exportBits(), splitWordIndex, maxWordAmount + 1);

				if (splitWordIndex >= B.MSW) {
					*this = ((highA * B) << (32*splitWordIndex)) + (lowA * B);
					return *this;
				}

				int_limited lowB;
				lowB.importBits(B.exportBits(), 0, splitWordIndex);
				int_limited highB;
				highB.importBits(B.exportBits(), splitWordIndex, maxWordAmount + 1);

				int_limited z0 = lowA * lowB;
				int_limited z2 = highA * highB;
				int_limited z1 = (lowA + highA) * (highB + lowB) - z0 - z2;

				*this = z2;
				*this <<= 32*splitWordIndex;
				*this += z1;
				*this <<= 32*splitWordIndex;
				*this += z0;
				this->updateLSW(A.LSW);
				this->updateMSW(A.MSW + B.MSW + 1);
				return *this;
			}
			int_limited operator* (int_limited const& rhs) const {
				int_limited result = *this;
				return result *= rhs;
			}


			// Considering the implementation of bitshifting, negation and addition with MSW, LSW
			// This division should have a complexity of O(bitSize + (rhs.MSW - rhs.LSW)^2)
			int_limited& operator/= (int_limited const& rhs) {
				if (rhs == 0) throw std::domain_error("Divide by zero exception");

				// Create the dividend and divisor with at least an extra word of accuracy for indexing in the algorithm
				const int extraPrecision = 32 + 32-bitSize%32;
				int_limited<bitSize + extraPrecision> dividend;
				int_limited<bitSize + extraPrecision> divisor;
				dividend.importBits(this->exportBits());
				divisor.importBits(rhs.exportBits());
				
				// Convert both of the *original* values to positive
				// We don't need to worry about the asymmetry of two's complement integer limits
				// because of the extra precision
				bool negative = false;
				if (*this < 0) {
					dividend = ((~dividend + 1) << extraPrecision) >> extraPrecision;
					negative = !negative;
				}
				if (rhs < 0) {
					divisor = ((~divisor + 1) << extraPrecision) >> extraPrecision;
					negative = !negative;
				}

				*this = 0;
				if (divisor > dividend) {
					return *this;
				}

				int potentialLSW = dividend.LSW - divisor.MSW - 1;
				int potentialMSW = dividend.MSW - divisor.MSW + 1;
				
				// If the divisor only consists of one 32-bit word
				// then divide manually and return
				if (divisor.MSW == 0) {
					uint32_t divWord = divisor.words[0];
					uint32_t rem = 0;
					for (int curInd = dividend.MSW; curInd >= 0; curInd--) {
						uint64_t curWord = concatTo64Bit(rem, dividend.words[curInd]);
						this->words[curInd] = curWord / divWord;
						rem = curWord % divWord;
					}
					this->updateLSW(potentialLSW);
					this->updateMSW(potentialMSW);
					if (negative) *this = ~(*this) + 1;
					return *this;
				}
				// This is an implementation of the division algorithm described
				// in pages 272-273 in Knuth's Art of Computer Programming - Volume 2
				// u represents the dividend, v the divisor, q the quotient

				
				// vInd is guaranteed to be less than the index of the most significant word with the extended precision values
				int vInd = divisor.MSW;
				// Specifically requires pre-shift values, so that m + vInd + 1 is within bounds
				int m = dividend.MSW - vInd;

				// the most significant word of v has to be at least 31 bits
				// the following code counts the number of leading zeros
				// inspired by https://stackoverflow.com/revisions/66486689/4
				uint32_t copyDiv = divisor.words[vInd];
				int shiftCount = 32;
				for (int curShift = 16; curShift > 1; curShift/=2) {
					uint32_t y = copyDiv >> curShift;
					if (y != 0) {
						shiftCount -= curShift;
						copyDiv = y;
					}
				}
				if (copyDiv != 0) shiftCount -= 1 + (copyDiv > 1);

				// the original divisor is also shifted to help calculate the remainder faster
				if (shiftCount != 0) {
					divisor <<= shiftCount;
					dividend <<= shiftCount;
				}
				// we never manually update dividend MSW or LSW after this, because no operations are based on them
				
				for (int j = m; j >= 0; j--) {
					uint64_t curDigits = concatTo64Bit(dividend.words[j + vInd + 1], dividend.words[j + vInd]);

					// quotient estimate and remainder
					uint64_t qEst = curDigits / divisor.words[vInd];
					uint64_t rem = curDigits - qEst * divisor.words[vInd];
					// qEst should be at most (UINT32_MAX + 1)
					if (qEst > UINT32_MAX ||
					(qEst * divisor.words[vInd - 1]) > concatTo64Bit(rem, dividend.words[j + vInd - 1])) {
						qEst -= 1;
						rem += divisor.words[vInd];
						// repeat the test
						// (rem > UINT32_MAX results in failure, but is also out of precision)
						if (rem <= UINT32_MAX && (qEst * divisor.words[vInd - 1]) > concatTo64Bit(rem, dividend.words[j + vInd - 1])) {
							qEst -= 1;
							rem += divisor.words[vInd];
						}
					}
					int_limited<bitSize + extraPrecision> diff = divisor * qEst;

					// Subtract diff manually, since the values have the same precision, but are offset
					// Also because we aren't supposed to propagate the borrow all the way to the left
					bool borrow = false;
					for (int curInd = diff.LSW; curInd <= vInd + 1; curInd++) {
						bool flag1 = diff.words[curInd] > dividend.words[j + curInd];
						bool flag2 = (diff.words[curInd] == dividend.words[j + curInd]) && borrow;
						dividend.words[j + curInd] -= diff.words[curInd] + borrow;
						borrow = flag1 || flag2;
					}
					this->words[j] = uint32_t(qEst);

					// If the result is negative, add the divisor back once
					if (borrow) {
						this->words[j] -= 1;
						// add the divisor back manually, because of the offset
						// This is where extending the divisor's precision by one word helps
						bool carry = false;
						for (int curInd = divisor.LSW; curInd <= vInd + 1; curInd++) {
							char flag1 = (divisor.words[curInd] >= BIT32_ON) + (dividend.words[j + curInd] >= BIT32_ON);
							dividend.words[j + curInd] += divisor.words[curInd] + carry;
							bool flag2 = dividend.words[j + curInd] < BIT32_ON;
							carry = (flag1 + flag2) > 1;
						}
						// The last carry here cancels out the last borrow from before
					}
				}

				this->updateLSW(potentialLSW);
				this->updateMSW(potentialMSW);
				if (negative) *this = ~(*this) + 1;
				return *this;
			}
			int_limited operator/ (int_limited const& rhs) const {
				int_limited result = *this;
				return result /= rhs;
			}

			// the sign of the divisor *does not* affect the result
			int_limited& operator%= (int_limited const& rhs) {
				if (rhs == 0) throw std::domain_error("Modulo by zero exception");
				
				// Create the dividend and divisor with at least an extra word of accuracy for indexing in the algorithm
				const int extraPrecision = 64 + 32-bitSize%32;
				int_limited<bitSize + extraPrecision> dividend;
				int_limited<bitSize + extraPrecision> divisor;
				dividend.importBits(this->exportBits());
				divisor.importBits(rhs.exportBits());
				
				// Convert both of the *original* values to positive
				// We don't need to worry about the asymmetry of two's complement integer limits
				// because of the extra precision
				bool negative = false;
				if (*this < 0) {
					dividend = ((~dividend + 1) << extraPrecision) >> extraPrecision;
					negative = !negative;
				}
				if (rhs < 0) divisor = ((~divisor + 1) << extraPrecision) >> extraPrecision;

				if (divisor > dividend) {
					// no need to change the sign, because we didn't change (*this) yet
					return *this;
				}
				*this = 0;

				int potentialLSW = 0;
				int potentialMSW = divisor.MSW;
				
				// If the divisor only consists of one 32-bit word
				// then divide manually and return
				if (divisor.MSW == 0) {
					uint32_t divWord = divisor.words[0];
					uint64_t curWord = 0;
					for (int curInd = dividend.MSW; curInd >= 0; curInd--) {
						curWord = concatTo64Bit(curWord % divWord, dividend.words[curInd]);
					}
					this->words[0] = curWord % divWord;
					this->updateLSW(potentialLSW);
					this->updateMSW(potentialMSW);
					if (negative) *this = ~(*this) + 1;
					return *this;
				}
				// This is an implementation of the division algorithm described
				// in pages 272-273 in Knuth's Art of Computer Programming - Volume 2
				// u represents the dividend, v the divisor, q the quotient

				
				// vInd is guaranteed to be less than the index of the most significant word with the extended precision values
				int vInd = divisor.MSW;
				// Specifically requires pre-shift values, so that m + vInd + 1 is within bounds
				int m = dividend.MSW - vInd;

				// the most significant word of v has to be at least 31 bits
				// the following code counts the number of leading zeros
				// inspired by https://stackoverflow.com/revisions/66486689/4
				uint32_t copyDiv = divisor.words[vInd];
				int shiftCount = 32;
				for (int curShift = 16; curShift > 1; curShift/=2) {
					uint32_t y = copyDiv >> curShift;
					if (y != 0) {
						shiftCount -= curShift;
						copyDiv = y;
					}
				}
				if (copyDiv != 0) shiftCount -= 1 + (copyDiv > 1);

				// the original divisor is also shifted to help calculate the remainder faster
				if (shiftCount != 0) {
					divisor <<= shiftCount;
					dividend <<= shiftCount;
				}
				
				for (int j = m; j >= 0; j--) {
					uint64_t curDigits = concatTo64Bit(dividend.words[j + vInd + 1], dividend.words[j + vInd]);

					// quotient estimate and remainder
					uint64_t qEst = curDigits / divisor.words[vInd];
					uint64_t rem = curDigits - qEst * divisor.words[vInd];
					// qEst should be at most (UINT32_MAX + 1)
					if (qEst > UINT32_MAX ||
					(qEst * divisor.words[vInd - 1]) > concatTo64Bit(rem, dividend.words[j + vInd - 1])) {
						qEst -= 1;
						rem += divisor.words[vInd];
						// repeat the test
						// (rem > UINT32_MAX results in failure, but is also out of precision)
						if (rem <= UINT32_MAX && (qEst * divisor.words[vInd - 1]) > concatTo64Bit(rem, dividend.words[j + vInd - 1])) {
							qEst -= 1;
							rem += divisor.words[vInd];
						}
					}
					int_limited<bitSize + extraPrecision> diff = divisor * qEst;

					// Subtract diff manually, since the values have the same precision, but are offset
					// Also because we aren't supposed to propagate the borrow all the way to the left
					bool borrow = false;
					for (int curInd = diff.LSW; curInd <= vInd + 1; curInd++) {
						bool flag1 = diff.words[curInd] > dividend.words[j + curInd];
						bool flag2 = (diff.words[curInd] == dividend.words[j + curInd]) && borrow;
						dividend.words[j + curInd] -= diff.words[curInd] + borrow;
						borrow = flag1 || flag2;
					}

					// If the result is negative, add the divisor back once
					if (borrow) {
						// add the divisor back manually, because of the offset
						// This is where extending the divisor's precision by one word helps
						bool carry = false;
						for (int curInd = divisor.LSW; curInd <= vInd + 1; curInd++) {
							char flag1 = (divisor.words[curInd] >= BIT32_ON) + (dividend.words[j + curInd] >= BIT32_ON);
							dividend.words[j + curInd] += divisor.words[curInd] + carry;
							bool flag2 = dividend.words[j + curInd] < BIT32_ON;
							carry = (flag1 + flag2) > 1;
						}
						// The last carry here cancels out the last borrow from before
					}
				}
				// only update MSW and LSW at the end, because it isn't needed in the middle
				dividend.updateLSW(potentialLSW);
				dividend.updateMSW(potentialMSW);
				dividend >>= shiftCount;
				for (int i = 0; i <= vInd; i++) {
					this->words[i] = dividend.words[i];
				}
				this->updateLSW(potentialLSW);
				this->updateMSW(potentialMSW);
				if (negative) *this = ~(*this) + 1;
				return *this;
			}
			int_limited operator% (int_limited const& rhs) const {
				int_limited result = *this;
				return result %= rhs;
			}
			#pragma endregion Arithmetic

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
			#pragma region Bitwise

			int_limited& operator^= (int_limited const& rhs) {
				for (int i = rhs.LSW; i <= rhs.MSW; i++) {
					this->words[i] ^= rhs.words[i];
				}
				this->updateLSW(std::min(this->LSW, rhs.LSW));
				this->updateMSW(std::max(this->MSW, rhs.MSW));
				return *this;
			}
			int_limited operator^ (int_limited const& rhs) const {
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
			int_limited operator| (int_limited const& rhs) const {
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
			int_limited operator& (int_limited const& rhs) const {
				int_limited result = *this;
				return result &= rhs;
			}

			// Returns the bit NOT, so adding 1 gets the two's complement
			int_limited operator~ () const {
				int_limited result = 0;
				for (int i = 0; i < this->wordCount; i++) {
					result.words[i] = ~this->words[i];
				}
				result.updateLSW(0);
				result.updateMSW(this->wordCount-1);
				return result;
			}

			// Classic non-arithmetic bitshift
			int_limited& operator<<= (unsigned int const& rhs) {
				if (rhs < 0) throw std::domain_error("Negative bitshift value exception");
				if (rhs == 0) return *this;
				int wordshift = rhs / 32;
				int bitshift = rhs % 32;
				for (int i = this->MSW; i >= this->LSW; i--) {
					this->wordShiftLeft(i, wordshift);
					this->bitShiftLeft(i + wordshift, bitshift);
				}
				this->updateLSW(this->LSW + wordshift - 1);
				this->updateMSW(this->MSW + wordshift + 1);
				return *this;
			}
			// Classic non-arithmetic bitshift
			int_limited operator<< (unsigned int const& rhs) const {
				int_limited result = *this;
				return result <<= rhs;
			}

			// Classic non-arithmetic bitshift
			int_limited& operator>>= (unsigned int const& rhs) {
				if (rhs < 0) throw std::domain_error("Negative bitshift value exception");
				if (rhs == 0) return *this;
				int wordshift = rhs / 32;
				int bitshift = rhs % 32;
				for (int i = this->LSW; i <= this->MSW; i++) {
					this->wordShiftRight(i, wordshift);
					this->bitShiftRight(i - wordshift, bitshift);
				}
				this->updateLSW(this->LSW - wordshift - 1);
				this->updateMSW(this->MSW - wordshift + 1);
				return *this;
			}
			// Classic non-arithmetic bitshift
			int_limited operator>> (unsigned int const& rhs) const {
				int_limited result = *this;
				return result >>= rhs;
			}
			#pragma endregion Bitwise


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
			#pragma region Relational

			bool operator== (int_limited const& rhs) const {
				// If they don't have 1's in the same words, return false
				if (this->LSW != rhs.LSW || this->MSW != rhs.MSW) return false;
				// Since they have the same word range, then just check the equality of those
				for (int i = this->LSW; i <= this->MSW; i++) {
					if (this->words[i] != rhs.words[i]) return false;
				}
				return true;
			}
			bool operator!= (int_limited const& rhs) const {
				return !(*this == rhs);
			}
			bool operator> (int_limited const& rhs) const {
				uint32_t MSb = BIT32_ON;
				if (bitSize%32 != 0) {
					MSb >>= 32 - (bitSize%32);
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
			bool operator< (int_limited const& rhs) const {
				uint32_t MSb = BIT32_ON;
				if (bitSize%32 != 0) {
					MSb >>= 32 - (bitSize%32);
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
			bool operator>= (int_limited const& rhs) const {
				return !(*this < rhs);
			}
			bool operator<= (int_limited const& rhs) const {
				return !(*this > rhs);
			}
			#pragma endregion Relational

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
			#pragma region Logical
			// returns *this == 0
			bool operator! () const {
				return this->MSW == 0 && this->words[0] == 0;
			}
			bool operator&& (int_limited const& rhs) const {
				// if *this and rhs are non-zero
				return (!!(*this)) && (!!(rhs));
			}
			bool operator|| (int_limited const& rhs) const {
				// if *this or rhs are non-zero
				return (!!(*this)) || (!!(rhs));
			}
			#pragma endregion Logical

			/*
			SECTION: MATHEMATICAL FUNCTIONS
			=============================================================
			ilog2() (integer binary log - floored)
			pow(e) (power)
			isqrt() (integer square root - floored)

			For now, every function returns the floor value, instead of a rounded value
			=============================================================
			*/
			#pragma region Math
			// returns a signed integer of the floored binary log
			int ilog2() const {
				if (*this <= 0) throw std::domain_error("Logarithm of non-positive exception");
				int result = this->MSW * 32;
				
				int leadingZeroes = 32;
				uint32_t copyMSW = this->words[this->MSW];
				for (int shift = 16; shift > 1; shift/=2) {
					uint32_t y = copyMSW >> shift;
					if (y != 0) {
						leadingZeroes -= shift;
						copyMSW = y;
					}
				}
				if (copyMSW != 0) leadingZeroes -= 1 + (copyMSW > 1);
				return result + 32-leadingZeroes;
			}

			// only accepts *positive* exponents
			int_limited pow(uint32_t exp) const {
				if (exp == 0) return 1;
				int_limited result;
				if (exp == 1) {
					result = *this;
				} else {
					result = this->pow(exp/2) * this->pow(exp/2);
					if (exp%2) result *= *this;
				}
				return result;
			}

			// returns the floored value of integer square root
			int_limited isqrt() const {
				if (*this < 0) throw std::domain_error("Sqrt of negative value exception");
				if (this->MSW == 0 && this->words[0] < 4) return (this->words[0] > 0);
				int sigBits = this->ilog2();
				int_limited low = (*this >> (sigBits/2 + 1));
				int_limited high = (*this >> (sigBits/2 - 1));
				int_limited mid = (low + high) >> 1;
				while (mid != low) {
					if (mid.pow(2) > *this) high = mid;
					else if (mid.pow(2) < *this) low = mid;
					else break;
					mid = (low + high) >> 1;
				}
				return mid;
			}

			#pragma endregion Math
	};
}