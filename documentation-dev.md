For an overview of the capabilities of every class, see the [User Documentation](documentation-user.md)

# Table of Contents <!-- omit in toc -->

- [Terminology](#terminology)
- [About the library](#about-the-library)
- [Integer values](#integer-values)
	- [int128](#int128)
		- [Construction and type casting](#construction-and-type-casting)
		- [Arithmetic operators](#arithmetic-operators)
		- [Bit operators](#bit-operators)
		- [Relational operators](#relational-operators)
		- [Logical operators](#logical-operators)
	- [int\_limited](#int_limited)
		- [Helper functions](#helper-functions)
			- [truncateExtraBits](#truncateextrabits)
			- [updateLSW](#updatelsw)
			- [updateMSW](#updatemsw)
			- [wordShiftLeft](#wordshiftleft)
			- [wordShiftRight](#wordshiftright)
			- [bitShiftLeft](#bitshiftleft)
			- [bitShiftRight](#bitshiftright)
			- [basicMult](#basicmult)
		- [Construction and type casting](#construction-and-type-casting-1)
			- [importBits](#importbits)
			- [exportBits](#exportbits)
		- [Arithmetic operators](#arithmetic-operators-1)
		- [Bit operators](#bit-operators-1)
		- [Relational operators](#relational-operators-1)
		- [Logical operators](#logical-operators-1)
- [Testing](#testing)
	- [Notes about the boost multiprecision library](#notes-about-the-boost-multiprecision-library)

# Terminology

In this text, `word` is to be understood as a 64 bit unsigned integer, which is the standard machine word size for a processor at the time of creation.

When discussing time or space complexity, $N$ refers to the amount of bits the terms for the operator have (e.g. for `int_limited<bitSize>`: `N == bitSize`).

# About the library

All of the library content should be under the namespace `largeNumberLibrary` (name subject to change).

The main envisioned use case of this library is the ability to use large bit numerical values (both integers and floating point), without having to download and install a large robust library containing other modules, like boost.

# Integer values

All integer values should be saved in unsigned 64 bit integers, stored in little endian order (least significant word first).
Casting an integer value to a standard library integer type should only return the least significant word cast into the type, to allow for easier bit manipulation.

## int128

`int128` is made to represent a 128 bit signed integer in two's complement representation.
This class should be kep relatively simple, however there are a few functions that should still be implemented, before this class can be considered truly complete:
- Conversion *from* `std::string` to `int128`
- Extraction from stream

Class sections:
<!-- no toc -->
- [Construction and type casting](#construction-and-type-casting)
- [Arithmetic operators](#arithmetic-operators)
- [Bit operators](#bit-operators)
- [Relational operators](#relational-operators)
- [Logical operators](#logical-operators)

### Construction and type casting

Currently, construction from two unsigned integers is done with the most significant word first.
This is the be changed in the future, to be more consistent with other classes, which start with least significant word first.

### Arithmetic operators

Addition is implemented with the addition of each 64 bit word and subtraction simply negates the right hand side of the operator and then calls addition.
It may be possible to slightly optimize subtraction by implementing it similarly to addition (mainly saving time on creating a copy of the value), however no changes should be made without testing and benchmarking.

Multiplication, division and modulo are all implemented with shift-addition, resulting in a quadratic complexity based on the number of bits. Due to the relatively small integer values, a asymptotically optimal algorithm would only be slower, due to a large amount of constants, so the only changes made here should be a constant optimization of the current algorithm.

Every arithmetic operator should be implemented first as its compound operator (e.g. `+=`), then the basic operator simply create a copy of the left hand side variable and use the compound operator on the copy.
Example:
```cpp
int128 operator/(int128 const& rhs) {
	int128 result(B1, B0);
	return result /= rhs;
}
```
The only exception to this is subtraction, where the basic operator may as well directly call compound addition, after negating the right hand side.

### Bit operators

Every bit operator acts upon the whole value without any regard to its numerical value. The operator `~` is implemented as a bit NOT, not as a complement operator.

Bit-shifting only accepts positive values.
It also defines bit-shifting for values larger than or equal to the integers bit size.
In those cases every bit is simply set to zero.

### Relational operators

Nothing much is to be said here. Their behaviour is equal to their behaviour for standard library integers.

### Logical operators

These operators are to be treated as simple ways to check if a value is non-zero or zero.

## int_limited

`int_limited<int bitSize>` is a template class made to represent an arbitrary, fixed bit signed integer in two's complement representation.
The value of the integer itself is held in an a vector of 64 bit unsigned integers, starting from the least significant word, ending with the most significant word.
The vector's length is the minimum required to hold the set number of bits.
If the set bit size is not a multiple of 64, then any extra bits are truncated by `truncateExtraBits()`, which is called at the end of `updateMSW()`.
For more information, see the [truncateExtraBits](#truncateextrabits) section.

Every instance has `wordCount` defined as the length of the vector of words.
Furthermore, every instance has `MSW` and `LSW` defined, which hold the index of the most significant word and the least significant word respectively.
These variable are imperative to optimizing ALL operators, so that they do not have to iterate through every word, when we know which of them are definitely zero.

Due to the usage of these variables in every operator, every function which manipulates any of the bits in the words is required to update both variables `MSW` and `LSW`, by calling `updateMSW(lowerBound)` and `updateLSW(upperBound)`.
More about these functions can be found out in the [Helper Functions](#helper-functions) section.

Here are some functions that are yet to be implemented:
- Conversion *from* `std::string` to `int_limited`
- Extraction from stream

Class sections:
<!-- no toc -->
- [Helper functions](#helper-functions)
	- [truncateExtraBits](#truncateextrabits)
	- [updateLSW](#updatelsw)
	- [updateMSW](#updatemsw)
	- [wordShiftLeft](#wordshiftleft)
	- [wordShiftRight](#wordshiftright)
	- [bitShiftLeft](#bitshiftleft)
	- [bitShiftRight](#bitshiftright)
	- [basicMult](#basicmult)
- [Construction and type casting](#construction-and-type-casting-1)
	- [importBits](#importbits)
	- [exportBits](#exportbits)
- [Arithmetic operators](#arithmetic-operators-1)
- [Bit operators](#bit-operators-1)
- [Relational operators](#relational-operators-1)
- [Logical operators](#logical-operators-1)

### Helper functions

Helper functions are private functions that are to be used for internally defined operators and functions.

#### truncateExtraBits

This function takes no arguments.

This function calculates how many bits in the most significant word are to be left alone and then it sets the other bits to zero.
It is called automatically at the end of `updateMSW()` if the most significant word contains a non-zero value.
It is unnecessary to call this function individually after calling `updateMSW()`.

If this function sets all of the bits in the most significant word to zero and the integer has more than one word, then `updateMSW()` is called again.
However in this case, `truncateExtraBits()` will most definitely not be called again.

#### updateLSW

Arguments:
- `int lowerBound` (`default = 0`)

This function takes an optional argument as a lower-bound and starts iterating over every word, starting from the lower-bound, continuing up, until it reaches a non-zero word.
The index of that word is then set as the value of `LSW`.
If all words above the lower-bound are zero, then the value of `LSW` is set to zero

This function should only be passed an argument if it is certain that the lower-bound is correct.
If unsure, simply reduce the lower-bound until you are certain.
The lower-bound is range checked and always set within the bounds of the possible value (i.e. `0` to `this->wordCount`), so no segmentation faults should occur.

#### updateMSW

Arguments:
- `int upperBound` (`default: this->wordCount - 1`)

This function takes an optional argument as an upper-bound and starts iterating over every word, starting from the upper-bound, continuing down, until it reaches a non-zero word.
The index of that word is then set as the value of `MSW`.
if all word below the upper-bound are zero, then the value of `MSW` is set to zero.

This function should only be passed an argument if it is certain that the upper-bound is correct.
If unsure, simply increase the upper-bound until you are certain.
The upper-bound is range checked and always set within the bounds of the possible value (i.e. `0` to `this->wordCount`), so no segmentation faults should occur.

#### wordShiftLeft

Arguments:
- `int wordIndex`
- `int shift`

This function takes the index of a word and the amount of **words** to shift by and it shifts ONLY the word pointed to by `wordIndex` in the direction of the most significant word.
If the new words position is within bounds, then the word at the new index is set to the value of the original word (its original bits are discarded).
Regardless of whether the new position was within bounds, the original word is set to zero.

Example:
```cpp
// this->words[5] == 540, this->words[7] == 3
this->wordShiftLeft(5, 2);
// this->words[5] == 0, this->words[7] == 540
```


#### wordShiftRight

Arguments:
- `int wordIndex`
- `int shift`

This function takes the index of a word and the amount of **words** to shift by and it shifts ONLY the word pointed to by `wordIndex` in the direction of the least significant word.
If the new words position is within bounds, then the word at the new index is set to the value of the original word (its original bits are discarded).
Regardless of whether the new position was within bounds, the original word is set to zero.

Example:
```cpp
// this->words[5] == 540, this->words[3] == 3
this->wordShiftRight(5, 2);
// this->words[5] == 0, this->words[3] == 540
```

#### bitShiftLeft

Arguments:
- `int wordIndex`
- `int shift`

This function takes the index of a word and the amount of **bits** to shift it by and it shifts ONLY the word pointed to by `wordIndex` in the direction of the most significant word.
Any bits that are shifted out of the original word are bit ORed into the next more significant word.

Example:
```cpp
// this->words[5] == UINT64_MAX, this->words[6] == 1024
this->bitShiftLeft(5, 2);
// this->words[5] == (UINT64_MAX << 2), this->words[6] == 1024 | (UINT64_MAX >> 62)
```

#### bitShiftRight

Arguments:
- `int wordIndex`
- `int shift`

This function takes the index of a word and the amount of **bits** to shift it by and it shifts ONLY the word pointed to by `wordIndex` in the direction of the least significant word.
Any bits that are shifted out of the original word are bit ORed into the next less significant word.

Example:
```cpp
// this->words[5] == UINT64_MAX, this->words[4] == 3
this->bitShiftRight(5, 2);
// this->words[5] == (UINT64_MAX >> 2), this->words[4] == 3 | (UINT64_MAX << 62)
```

#### basicMult

Arguments:
- `int_limited const& A`
- `int_limited const& B`

This function set the value of `*this` to the value of the multiplication of `A` and `B`.
The multiplication is done by taking a word at index `b_i` from `B` and a word at index `a_i` from `A` and setting the value of `this->words[a_i + b_i]` to the result of the multiplication.

If the word at index `a_i + b_i` is the most significant word, then a carry for the next word is not set, otherwise the product of the two words is saved in an `int128`, where the lower 64 bits are added to the value of `this->words[a_i + b_i]` and the upper 64 bits are set as the carry for the next word.

The carry is stored in an `uint64_t`, however the largest possible carry is `UINT64_MAX + 1`, so the additional one is saved as a bool, which is set when the addition of the product into `this->words[a_i + b_i]` overflows.

At the end of the multiplication, `LSW` is updated with a lower bound of `A.LSW` and `MSW` is updated with an upper bound of `A.MSW + B.MSW + 1`.

The code was written as an extension of the pseudocode from the [Wikipedia page about multiplication](https://en.wikipedia.org/wiki/Multiplication_algorithm#Other_notations) and is used as a fall back from Karatsuba's algorithm during multiplication, when one of the arguments becomes small enough.

### Construction and type casting

Casting *from* standard library integers does not require setting `*this = 0`, because the vector is already initialised with zero's.
However it is important to remember to call `truncateExtraBits()` in case the bit size of the class is less than the original integer's size.
`LSW` and `MSW` are not updated, because their default values are zero.

Casting *to* standard library integers simply returns the least significant word cast as the required type, to allow for easier bit manipulation

#### importBits

Overload 1:
Arguments:
- `std::vector<uint64_t> newWords`

The first overload of importBits() simply takes the whole vector of newWords and will overwrite all of the words of `*this`, starting from the least significant word, until either the end of `newWords` or the end of the class instances words.
Any additional words in `newWords` will be ignored.
If any words weren't modified, their value will be set to zero.
Using this overload is the recommended way of casting a lower bit `int_limited` to a larger bit `int_limited`.

At the end, `LSW` is updated with a lower bound of `0` and `MSW` is updated with an upper bound of `newWords.size() - 1`.

Overload 2:
Arguments:
- `std::vector<uint64_t> newWords`
- `int startIndex`
- `int endIndex`
- `int wordOffset` (`default = 0`)

In this case, the bits will start to be imported from `newWords[startIndex]` to `newWords[endIndex - 1]`.
All words before and after the imported words will be set to zero.
A range error is thrown if any of the values are negative.

At the end, `LSW` is updated with a lower bound of `wordOffset` and `MSW` is updated with an upper bound of `endIndex - startIndex + wordOffset + 1` (the amount of imported words `+ wordOffset`).

Overload 3:
Arguments:
- `std::vector<uint64_t>::iterator beginWords`
- `std::vector<uint64_t>::iterator endWords`
- `int wordOffset` (`default = 0`)

The third overload accepts iterators over the presumed `newWords` vector, overwriting bits from first iterator to the (non-inclusive) second iterator.
Additionally, there is the option to set the `wordOffset`.
Any non-modified words (even those before `wordOffset`) will be set to zero.
A range error is thrown if `wordOffset` is negative.

At the end, `LSW` is updated with a lower bound of `wordOffset` and `MSW` is updated with an upper bound of `index - 1`, where `index` is an internal counter which remembers the index of the last word that was overwritten.


#### exportBits

Simply returns the whole array of words from `*this`.

### Arithmetic operators

Addition and subtraction are rather straight-forward in their linear algorithm of adding words and setting a carry bit.

Multiplication makes use of Karatsuba's algorithm, allowing for slight better asymptotic complexity when multiplying large values.
The algorithm's complexity is $O(N^{\log_{2}3}) \approx O(N^{1.585})$, however due to rather large constants, whenever one of the factors is small enough (currently set as 8 words or less), then basic shift-addition multiplication takes place, by calling `basicMult()`.

The premise of Karatsuba's algorithm lies in the division of the bits of the number. For simplicities sake, the implementation doesn't divide the bits exactly in half, only the amount of words.

If the smaller factor has less than half of the words of the larger factor, then the division of the smaller factor's bits is skipped and smaller factor is directly multiplied with the shifted upper half and the lower half of the larger factor's bits.
In both this case and the case of `basicMult()`, `LSW` and `MSW` aren't directly updated, because  they are already updated when setting the value before returning `*this`.

Much of the inspiration about the method of implementation comes from [here](http://kt8216.unixcab.org/karatsuba/index.html), where the information provided helped rewrite the algorithm into a functional state.
The value of 8 words was also taken from there, so some benchmarking is required to experimentally verify that such a value is ideal for the current implementation as well.

Division and modulo both still rely on quadratic complexity shift-addition (more precisely subtraction) method, because it is not believed that the current multiplication is fast enough to allow for a more asymptotically efficient algorithm for division (which requires multiplication).

### Bit operators

Bit AND, OR, XOR and NOT are implemented by iterating over every word and using the operator for every pair of words from `*this` and the right hand side.
Bit-shifting is done by calculating the amount of word-shifts and bit-shifts and then calling their respective functions for every word.
For shifting to the most significant word, this method starts from the most significant bit and then continues for less significant words, so as to not overwrite words that have not been shifted yet. The same applies for shifting to the least significant word, where the shifting start from the least significant word and continues to the most significant word.

### Relational operators

Equality makes use of the fact that both `MSW` and `LSW` must be equal for both values, thus saving time by not always having to check every word between them.

Comparison operators are implemented in a similar way, but first they have to calculate the most significant bit of the most significant word, so that they can uncover the signs of the values, thus only comparing `MSW` when both values have the same sign (otherwise negative values have a larger `MSW` by default).

The slowest factor of these operations when comparing with standard library integers is the conversion to an `int_limited`.
If only checking for whether the value is zero or not, using logical operators is recommended.

### Logical operators

These operators simply return whether a value is zero or non-zero.
Note that since implicit conversion from `int_limited` to `bool` is not allowed, usage of a class instance by itself as a condition is not possible. The recommended method of checking if a number is non-zero is `example != 0` or `!!example` if speed is required.

# Testing

The correctness of each class is to be tested by comparing with the boost multiprecision library.
The process of testing requires carefulness to make sure that the values generated for both libraries contain the same bits and are numerically the same value in both representations.

To check out the methods used, it is recommended to look at [testing-int_limited/main.cpp](testing-int_limited/main.cpp).

## Notes about the boost multiprecision library

The boost multiprecision library allows importing bits into an integer value, however the bits accepted are to be most significant word first (as compared to this libraries least significant word first)

Casting from a negative boost integer value to an unsigned value throws an error, so casting to a signed value is required.

Note that, although boost saves its integers in two's complement, the sign bit can be considered an additional hidden bit, allowing values from $2^{128}-1$ to $-(2^{128}-1)$.
Additionally, bit-shifting is arithmetic, meaning that the sign is kept from the original value.