# Table of Contents <!-- omit in toc -->

- [Terminology](#terminology)
- [Usage](#usage)
- [int128.hpp](#int128hpp)
	- [int128](#int128)
		- [Class construction, conversion to standard types and std::string](#class-construction-conversion-to-standard-types-and-stdstring)
			- [Type Casting](#type-casting)
			- [Conversion to std::string](#conversion-to-stdstring)
		- [Arithmetic operators](#arithmetic-operators)
		- [Bit operators](#bit-operators)
		- [Relational operators](#relational-operators)
		- [Logical operators](#logical-operators)
		- [Static functions](#static-functions)
			- [className()](#classname)
- [int\_limited.hpp](#int_limitedhpp)
	- [int\_limited](#int_limited)
		- [Notes about the class](#notes-about-the-class)
		- [Class construction, conversion to standard types and std::string](#class-construction-conversion-to-standard-types-and-stdstring-1)
			- [Importing and exporting bits](#importing-and-exporting-bits)
			- [Type casting](#type-casting-1)
			- [Conversion to std::string](#conversion-to-stdstring-1)
		- [Arithmetic operators](#arithmetic-operators-1)
		- [Bit operators](#bit-operators-1)
		- [Relational operators](#relational-operators-1)
		- [Logical operators](#logical-operators-1)
		- [Static functions](#static-functions-1)
			- [className()](#classname-1)
- [PLACEHOLDER HEADER FOR BENCHMARKS](#placeholder-header-for-benchmarks)


# Terminology

In this text, `word` is to be understood as a 64 bit unsigned integer, which is the standard machine word size for a processor at the time of creation.

When discussing time or space complexity, $N$ refers to the amount of bits the terms for the operator have (e.g. for `int_limited<bitSize>`: `N == bitSize`).

# Usage

Currently, each class is in its own separate `.hpp` files, so all that is required is downloading the files locally and including the desired header.
I am aware that best practice states that header files should only contain declarations, however any further changes to the library structure will wait until a concrete plan is made.

Every variable, types and classes are defined in the namespace `largeNumberLibrary`. \
Currently, there exists 2 header files:
- [int128.hpp](#int128hpp)
- [int_limited.hpp](#int_limitedhpp)

# int128.hpp

`int128.hpp` defines the class `int128` and two constants `BIT64_ON` and `UINT63_MAX`. \
`BIT64_ON` and `UINT63_MAX` equivalent to the standard library's `INT64_MIN` and `INT64_MAX` respectively, defined in the header `stdint.h`.

## int128

This class provides a functional 128 bit signed integer represented in two's complement.
All bit operators (bit NOT, AND, OR, XOR, bit-shifting), relational operators (equality and inequality), arithmetic operators (addition, subtraction, multiplication, division, modulo) and logical operators (logical NOT, AND, OR) are implemented.

Additionally, every instance has access to the member function `toString()` (for more, checkout [Class construction, conversion to standard types and std::string](#class-construction-conversion-to-standard-types-and-stdstring))

Class sections:
<!-- no toc -->
- [Class construction, conversion to standard types and std::string](#class-construction-conversion-to-standard-types-and-stdstring)
	- [Type Casting](#type-casting)
	- [Conversion to std::string](#conversion-to-stdstring)
- [Arithmetic operators](#arithmetic-operators)
- [Bit operators](#bit-operators)
- [Relational operators](#relational-operators)
- [Logical operators](#logical-operators)
- [Static functions](#static-functions)
	- [className()](#classname)


### Class construction, conversion to standard types and std::string

#### Type Casting

Class construction can be implicit from 32 or 64 bit standard library integers (`uint64_t`, `int64_t`, `unsigned int`, `int`) or by passing the most significant word and the least significant word in the constructor.

Examples:

```cpp
using namespace largeNumberLibrary;

int128 example1 = 0;
int128 example2 = UINT64_MAX;
// Equivalent to -1, due to two's complement
int128 example3(UINT64_MAX, UINT64_MAX);
```

This means that implicit conversion to `int128` is allowed for easy comparison (e.g.`example2 != 0`).
However conversion from `int128` to a standard type is strictly explicit, requiring explicit casting (e.g. `int zero = (int)example1`).

Such explicit casting simply returns the bits in the least significant word and casts them to the respective type. \
For example:
```cpp
int128 num = -1;
uint64_t truncatedNum = (uint64_t)num;
// will return true, because of the two's complement representation of num
assert(truncatedNum == UINT64_MAX);
```

Explicit casting is allowed from `int128` to `uint64_t`, `int64_t`, `unsigned int`, `int` and `char`.

Note that since implicit conversion from `int128` to `bool` is not allowed, usage of a class instance by itself as a condition is not possible. The recommended method of checking if a number is non-zero is `example != 0` or `!!example`.

#### Conversion to std::string

Class instances can be converted to a string with the member function `toString(uint64_t base = 10)`. By default, it will convert the value into base 10, however an argument may be passed to convert to a different number base.
The size of the number base is limited to the maximum value of an unsigned 64 bit integer.

Number bases ranging from 1 to 36 (inclusive) will be converted with capital letters of the English alphabet substituting as digits 10 to 36. \
Larger number bases are represented by numbers in base 10, with each "digit" separated by a underscore.

If the value is negative, then a minus sign will be inserted to the start of the string (regardless of number base).

Direct insertion to an output stream is also allowed, simply converting the value to base 10.

Examples:

```cpp
using namespace largeNumberLibrary;

int128 example4 = 0xDEADBEEF;
// returns "3735928559"
std::string base10 = example4.toString();
// returns "DEADBEEF";
std::string base16 = example4.toString(16);
// returns "11_47_37_21_21_9_"
std::string base50 = example4.toString(50);
// returns "-11011110101011011011111011101111"
std::string negativeBinary = (-example4).toString(2);
// prints "3735928559"
std::cout << example4;
```

### Arithmetic operators

The class supports all arithmetic operators and their respective compound operators (e.g. `+` and `+=`).
Simple numerical negation is also supported (e.g. `int128 negative = -example4`).

Addition and subtraction may be considered to have constant complexity, requiring simply two 64 bit additions.
However multiplication, division and modulation are considerably slower.

Multiplication is implemented by shift-adding. It makes use of pre-calculating bits that fit within exactly one 64 bit word, so only a maximum of 64 shift-additions may occur.

Division and modulo is implemented in a similar way, but it may reach a maximum of 128 shift-additions.
Note that the behaviour of both division and modulo are equivalent to the C++ standard library and the boost multiprecision library. This means that division truncates towards zero and modulo keeps the sign of the dividend (left hand side).

### Bit operators

All bit operations behave mostly equivalently to the C++ standard library (i.e. `~` is defined as bit NOT, so adding 1 is equivalent to getting the values two's complement).

The only two differences in behaviour is that bit-shifting by 128 bits or larger values is defined as setting every bit to zero, as compared to C++ undefined behaviour when bit-shifting by an integers size and C++ allows shifting by a negative value (with a warning), whereas `int128` does not support it.

Note, that the boost multiprecision library overloads the bit-shift operators with arithmetic bit-shifting.

The complexity of these operations may be considered constant due to the integer's relatively small size.

### Relational operators

All relational operators (`==`, `!=`, `>`, `>=`, `<`, `<=`) are defined and behave equivalently to the C++ standard library.

The complexity of these operations may be considered constant due to the integer's relatively small size.

### Logical operators

These include the logical NOT (`!`), AND (`&&`) and OR (`||`).
Logical NOT return `true` if the value is zero.
logical AND and OR return `true` if (AND/OR both values) are non-zero.

Their implementation keeps the short circuit behaviour of their standard library counterparts.

Note that since implicit conversion from `int128` to `bool` is not allowed, usage of a class instance by itself as a condition is not possible. The recommended method of checking if a number is non-zero is `example != 0` or `!!example`.

### Static functions

#### className()

Returns the namespace and the class name as a `std::string`.
For this class, it will be `"largeNumberLibrary::int128"`.

# int_limited.hpp

`int_limited.hpp` contains the template class `int_limited` and some predefined types `int256`, `int512` and `int1024`, each respectively equivalent to `int_limited<256>`, `int_limited<512>`, `int_limited<1024>`.

`int_limited.hpp` includes `int128.hpp`, so by including this header, you gain access to everything in `int128.hpp`.

## int_limited

The class provides an arbitrary, fixed size signed integer represented in two's complement.

All bit operators (bit NOT, AND, OR, XOR, bit-shifting), relational operators (equality and inequality), arithmetic operators (addition, subtraction, multiplication, division, modulo) and logical operators (logical NOT, AND, OR) are implemented.

Class sections:
<!-- no toc -->
- [Notes about the class](#notes-about-the-class)
- [Class construction, conversion to standard types and std::string](#class-construction-conversion-to-standard-types-and-stdstring-1)
	- [Importing and exporting bits](#importing-and-exporting-bits)
	- [Type casting](#type-casting-1)
	- [Conversion to std::string](#conversion-to-stdstring-1)
- [Arithmetic operators](#arithmetic-operators-1)
- [Bit operators](#bit-operators-1)
- [Relational operators](#relational-operators-1)
- [Logical operators](#logical-operators-1)
- [Static functions](#static-functions-1)
	- [className()](#classname-1)

### Notes about the class

This class is a template class, which accepts the bit size of the desired integer. Each class instance will is constructed of 64 bit unsigned integers as words. If the bit size is not a multiple of 64, then any additional bits outside of the desired bitSize stay zero.

Every class instance supports overflow and class instances of equal bit size may interact with each other.

Examples:
```cpp
// Only the first 37 bits will be set to 1 (two's complement)
int_limited<37> a = -1;
// returns true, because adding 1 overflows the 37th bit and the 38th bit is truncated
assert(a + 1 == 0);
int_limited<256> b = 100;
b += a // Will not compile
```

### Class construction, conversion to standard types and std::string

Similarly to `int128`, this class allows construction by implicit casting from 32 or 64 bit standard library integers.

To otherwise assign the value of every bit, the use of member functions `importBits()` is required.

#### Importing and exporting bits

`ImportBits()` is a member function of every instance of `int_limited`.
It allows overwriting of individual words of a class instance, starting from the least significant word.
Any unmodified word will be set to zero (thus behaviour wise, this is equivalent to using the assignment operator).
It has three possible overloads:

```cpp
void importBits(std::vector<uint64_t>& newWords);
void importBits(std::vector<uint64_t>& newWords, int startIndex, int endIndex, int wordOffset = 0);
void importBits(std::vector<uint64_t>::iterator beginWords, std::vector<uint64_t>::iterator endWords, int wordOffset = 0);
```

The first overload will overwrite all of the words of `*this`, starting from the least significant word, until either the end of `newWords` or the end of the class instances words.
Any additional words in `newWords` will be ignored.
If any words weren't modified, their value will be set to zero.
Using this overload is the recommended way of casting a lower bit `int_limited` to a larger bit `int_limited`.

The second overloads accepts 3 additional arguments: the `startIndex` for `newWords`, the `endIndex` for `newWords` and the `wordOffset` for the class instances words.

In this case, the bits will start to be imported from `newWords[startIndex]` to `newWords[endIndex - 1]`. If `wordOffset` is omitted, then the overwrite will start from the least significant bit, otherwise it will start from `wordOffset`.
Any non-modified words (even those before `wordOffset`) will be set to zero.
A range error is thrown if any of the values are negative.

The third overload accepts iterators over the presumed `newWords` vector, overwriting bits from first iterator to the (non-inclusive) second iterator.
Additionally, there is the option to set the `wordOffset`.
Any non-modified words (even those before `wordOffset`) will be set to zero.
A range error is thrown if `wordOffset` is negative.

`exportBits()` is a member function that returns all of the words saved in the class instance, as a vector of unsigned 64 bit integers (least significant word first).

For example, the following code splits the words of `num`, so that the less significant half is in `lowNum` and the most significant half is in `highNum`:
```cpp
int wordCount = 9;
int_limited<9*64> num;
std::vector<uint64_t> numWords = num.exportBits();

int_limited<9*64> lowNum;
int_limited<9*64> highNum;
lowNum.importBits(numWords, 0, wordCount/2 + 1);
highNum.importBits(numWords, wordCount/2 + 1, wordCount);
```

#### Type casting

Implicit casting into `int_limited` is allowed from 32 and 64 bit standard library integers (`uint64_t`, `int64_t`, `unsigned int`, `int`).

However conversion from `int128` to a standard type is strictly explicit, requiring explicit casting (e.g. `int zero = (int)example1`).

Such explicit casting simply returns the bits in the least significant word and casts them to the respective type. \
For example:
```cpp
int_limited<256> num = -1;
uint64_t truncatedNum = (uint64_t)num;
// will return true, because of the two's complement representation of num
assert(truncatedNum == UINT64_MAX);
```

Explicit casting is allowed from `int128` to `uint64_t`, `int64_t`, `unsigned int`, `int` and `char`.

Note that since implicit conversion from `int_limited` to `bool` is not allowed, usage of a class instance by itself as a condition is not possible. The recommended method of checking if a number is non-zero is `example != 0` or `!!example` if speed is required.


#### Conversion to std::string

Class instances can be converted to a string with the member function `toString(uint64_t base = 10)`. By default, it will convert the value into base 10, however an argument may be passed to convert to a different number base.
The size of the number base is limited to the maximum value of an unsigned 64 bit integer.

Number bases ranging from 1 to 36 (inclusive) will be converted with capital letters of the English alphabet substituting as digits 10 to 36. \
Larger number bases are represented by numbers in base 10, with each "digit" separated by a underscore.

If the value is negative, then a minus sign will be inserted to the start of the string (regardless of number base).

Direct insertion to an output stream is also allowed, simply converting the value to base 10.

Examples:

```cpp
using namespace largeNumberLibrary;

int_limited<192> example = 0xDEADBEEF;
// returns "3735928559"
std::string base10 = example.toString();
// returns "DEADBEEF";
std::string base16 = example.toString(16);
// returns "11_47_37_21_21_9_"
std::string base50 = example.toString(50);
// returns "-11011110101011011011111011101111"
std::string negativeBinary = (-example).toString(2);
// prints "3735928559"
std::cout << example;
```

### Arithmetic operators

The class supports all arithmetic operators and their respective compound operators (e.g. `+` and `+=`).
Simple numerical negation is also supported (e.g. `int_limited<192> negative = -example4`).

Addition and subtraction both have a time and space complexity of $O(N)$, where $N$ represents the bit size of the class instance. 

Multiplication is implemented as Karatsuba's algorithm, with an asymptotic time complexity of $O(N^{\log_{2}3}) \approx O(N^{1.585})$, this however comes with the cost of $O(N\log(N))$ space complexity with a constant of $\approx 10$, which may cause an unexpected slow down for larger numbers.

Division and modulo are both implemented as shift-addition with a time complexity of $O(N^2)$ and space complexity of $O(N)$. For a performance comparison with multiplication, see [PLACEHOLDER HEADER FOR BENCHMARKS](#placeholder-header-for-benchmarks).

The behaviour of division and modulo are equivalent to the C++ standard library and the boost multiprecision library. This means that division truncates towards zero and modulo keeps the sign of the dividend (left hand side).

### Bit operators

All bit operations behave mostly equivalently to the C++ standard library (i.e. `~` is defined as bit NOT, so adding 1 is equivalent to getting the values two's complement).

The only two differences in behaviour is that bit-shifting by 128 bits or larger values is defined as setting every bit to zero, as compared to C++ undefined behaviour when bit-shifting by an integers size and C++ allows shifting by a negative value (with a warning), whereas `int_limited` does not support it.

Note, that the boost multiprecision library overloads the bit-shift operators with arithmetic bit-shifting.

Both the time and space complexity of these operations is $O(N)$, though they are faster than addition. 

### Relational operators

All relational operators (`==`, `!=`, `>`, `>=`, `<`, `<=`) are defined and behave equivalently to the C++ standard library.

Both the time and space complexity of these operations is $O(N)$, though they are on average significantly faster.

### Logical operators

These include the logical NOT (`!`), AND (`&&`) and OR (`||`).
Logical NOT return `true` if the value is zero.
logical AND and OR return `true` if (AND/OR both values) are non-zero.

Their complexity is equivalent to those of [relational operators](#relational-operators-1).

Note that since implicit conversion from `int_limited` to `bool` is not allowed, usage of a class instance by itself as a condition is not possible. The recommended method of checking if a number is non-zero is `example != 0` or `!!example` if speed is required.

### Static functions

#### className()

Returns the namespace and the class name as a `std::string`.
For this class, it will be `"largeNumberLibrary::int_limited<bitSize>"`, where `bitSize` represents the chosen template's bit size.


# PLACEHOLDER HEADER FOR BENCHMARKS

Content will be added once benchmarking for `int_limited` is implemented.