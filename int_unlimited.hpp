#include <cstdint>
#include <iostream>
#include <string>
#include <vector>

namespace customBigInt {
	// Two's complement
	// Constructs a vector of 64 bit unsigned integers, dynamically sized, so as to never overflow
	class int_unlimited {
		private:
			// LSb first
			// The most significant word is the last one
			std::vector<uint64_t> words;

		public:
			/*
			SECTION: CONSTRUCTION
			=============================================================
			default constructor
			base constructor
			IMPLICIT conversion FROM:
				uint64_t
				int64_t
				int
				uint
			conversion TO:
				uint64_t
				int64_t
				int
				char
			= (assignment operator)
			importBits
			exportBits
			=============================================================
			*/


			// Accepts conversions from individual standard int types
			// To convert multiple integers of a type into an int_unlimited
			// It is required to declare an instance and call importBits

			int_unlimited() { }
			int_unlimited(uint64_t a) {
				words.push_back(a);
			}
			int_unlimited(unsigned int a) {
				words.push_back((uint64_t)a);
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
				return "customBigInt::int_unlimited";
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
			+ (addition)
			- (subtraction)
			* (multiplication)
			/ (division)
			% (modulus)
			respective compound operators (+=, -=, *=, /=, %=)
			=============================================================
			*/


			/*
			SECTION: BITWISE OPERATORS
			=============================================================
			^ (XOR)
			| (OR)
			& (AND)
			~ (complement)
			<< (shift left)
			>> (shift right)
			respective compound operators (^=, |=, &=, <<=, >>=)
			=============================================================
			*/


			/*
			SECTION: RELATIONAL OPERATORS
			=============================================================
			== (equality)
			!= (inequality)
			> (greater-than)
			< (less-than)
			>= (greater-than-or-equal-to)
			<= (less-than-or-equal-to)
			=============================================================
			*/

			/*
			SECTION: LOGICAL OPERATORS
			=============================================================
			! (NOT)
			&& (AND)
			|| (OR)

			> When overloaded, these operators get function call precedence,
			and short circuit behavior is lost
			> This shouldn't be a problem here, since the overload itself makes use of normal boolean && and ||
			=============================================================
			*/
	};
}