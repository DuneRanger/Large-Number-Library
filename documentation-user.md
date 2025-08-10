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
		- [Mathematical methods](#mathematical-methods)
		- [Static functions](#static-functions-1)
			- [className()](#classname-1)
- [Benchmarks](#benchmarks)


# Terminology

In this text, `word` is to be understood as the size of the integers making up the internal structure of the class.
For `int128` this will be 64 bits, whereas for `int_limited` it is 32 bits.

When discussing time or space complexity, $N$ refers to the amount of bits the terms for the operator have (e.g. for `int_limited<bitSize>`: `N == bitSize`).

# Usage

Currently, each class is in its own separate `.hpp` files, so all that is required is downloading the files locally and including the desired header.
I am aware that best practice states that header files should only contain declarations, however any further changes to the library structure will wait until a concrete plan is made.

Every variable, types and classes are defined in the namespace `largeNumberLibrary`. \
Currently, there exists 2 main header files:
- [int128.hpp](#int128hpp)
- [int_limited.hpp](#int_limitedhpp)

The documentation for any subdirectory of the library can be found within the subdirectory itself.

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

Class construction can be implicit from 32 or 64 bit standard library integers (`uint64_t`, `int64_t`, `unsigned int`, `int`), by passing the most significant word and the least significant word, or a string, into the constructor.

Examples:

```cpp
using namespace largeNumberLibrary;

int128 example1 = 0;
int128 example2 = UINT64_MAX;
// Equivalent to -1, due to two's complement
int128 example3(UINT64_MAX, UINT64_MAX);
// Equivalent to (1 << 64)
int128 example4(1, 0);
int128 example5("534531410382");
```

This means that implicit conversion to `int128` is allowed for easy comparison (e.g.`example2 != 0`). \
However conversion from `int128` to a standard type is strictly explicit, requiring explicit casting (e.g. `int zero = (int)example1`).

Note that construction from a string does not throw a warning or error, if the number in the string is larger than what a `int128` can contain.

Such explicit casting simply returns the bits in the least significant word and casts them to the respective type. \
For example:
```cpp
int128 num = -1; // Equivalent to all ones in the bits
uint64_t truncatedNum = (uint64_t)num;
// True, because of the two's complement representation of num
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

For examples of speed, see the [int128 benchmark results](./testing-int128/benchmark-results.md).
If we consider the speed of addition to be $1$, then subtraction is $2$, multiplication is $6$, and modulo and division around $65$)

Note that the behaviour of both division and modulo are equivalent to the C++ standard library and the boost multiprecision library. This means that division truncates towards zero and modulo keeps the sign of the dividend (left hand side).

### Bit operators

All bit operations behave mostly equivalently to the C++ standard library (i.e. `~` is defined as bit NOT, so adding 1 is equivalent to getting the values two's complement).

The only two differences in behaviour is that bit-shifting by 128 bits or larger values is defined as setting every bit to zero, as compared to C++ undefined behaviour when bit-shifting by an integers size. Additionally, C++ allows shifting by a negative value (with a warning), whereas `int128` does not support it.

Note, that the boost multiprecision library overloads the bit-shift operators with arithmetic bit-shifting,
so when comparing or switching libraries, pay attention when shifting a negative integer to the right.

### Relational operators

All relational operators (`==`, `!=`, `>`, `>=`, `<`, `<=`) are defined and behave equivalently to the C++ standard.

### Logical operators

These include the logical NOT (`!`), AND (`&&`) and OR (`||`).
Logical NOT return `true` if the value is zero.
logical AND and OR return `true` if (AND/OR both values) are non-zero.

Their implementation keeps the short circuit behaviour of their standard counterparts.

Note that since implicit conversion from `int128` to `bool` is not allowed, usage of a class instance by itself as a condition is not possible. The recommended method of checking if a number is non-zero is `example != 0` or `!!example`.

### Static functions

#### className()

Returns the namespace and the class name as a `std::string`.
For this class, it will be `"largeNumberLibrary::int128"`.

# int_limited.hpp

`int_limited.hpp` contains the template class `int_limited`, from which you can define types like:

```cpp
typedef int_limited<256> int256;
typedef int_limited<512> int512;
typedef int_limited<431> int431;
```

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

This class is a template class, which accepts the bit size of the desired integer. Each class instance will is constructed of 32 bit unsigned integers as words. If the bit size is not a multiple of 32, then any additional bits outside of the desired bitSize stay zero.
This does however induce an overhead to every operation (mainly addition and subtraction), so it is not recommended, unless the type already has more than 256 bits
The overhead is noticeable in the [benchmark graph](./testing-int_limited/benchmark-graph-int_limited.png), as the time jumps right after a multiple of 32, and the drops at the next multiple (e.g. 37 bits is slightly slower than 64 bits).

Every class instance supports overflow and class instances of equal bit size may interact with each other.

Examples:
```cpp
// Only the first 37 bits will be set to 1 (two's complement)
int_limited<37> a = -1;
// returns true, because adding 1 overflows the 37th bit and the 38th bit is truncated
assert(a + 1 == 0);
int_limited<256> b = 100;
b += a // Will compile
a += b // Will also compile, but only adds the lower 37 bits of b
```

### Class construction, conversion to standard types and std::string

Similarly to `int128`, this class allows construction by implicit casting from 32 or 64 bit standard library integers and from strings.

To otherwise assign the value of every bit, the use of the member functions `importBits()` is required.

#### Importing and exporting bits

`ImportBits()` is a member function of every instance of `int_limited`.
It allows overwriting of individual words of a class instance, starting from the least significant word.
Any unmodified word will be set to zero (thus behaviour wise, this is equivalent to using the assignment operator).
It has three possible overloads:

```cpp
void importBits(std::vector<uint32_t>& newWords);
void importBits(std::vector<uint32_t>& newWords, int startIndex, int endIndex, int wordOffset = 0);
void importBits(std::vector<uint32_t>::iterator beginWords, std::vector<uint32_t>::iterator endWords, int wordOffset = 0);
```

The first overload will overwrite all of the words of `*this`, starting from the least significant word, until either the end of `newWords` or the end of the class instances words.
Any additional words in `newWords` will be ignored.
If any words weren't modified, their value will be set to zero.
Using this overload is the recommended way of casting a lower bit `int_limited` to a larger bit `int_limited`.

The second overloads accepts 3 additional arguments: the `startIndex` for `newWords`, the `endIndex` for `newWords` and the `wordOffset` for the class instances words.

In this case, the bits will start to be imported from `newWords[startIndex]` to `newWords[endIndex - 1]`. If `wordOffset` is omitted, then the overwrite will start from the least significant bit, otherwise it will start from `wordOffset`.
Any non-modified words (even those before `wordOffset`) will be set to zero.
A range error is thrown if any index or offset is negative.

The third overload accepts iterators over the presumed `newWords` vector, overwriting bits from first iterator to the (non-inclusive) second iterator.
Additionally, there is the option to set the `wordOffset`.
Any non-modified words (even those before `wordOffset`) will be set to zero.
A range error is thrown if `wordOffset` is negative.

`exportBits()` is a member function that returns all of the words saved in the class instance, as a vector of unsigned 64 bit integers (least significant word first).

For example, the following code splits the words of `num`, so that the less significant half is in `lowNum` and the most significant half is in `highNum`:
```cpp
constexpr int w_count = 5;
int_limited<w_count*32> num;
std::vector<uint32_t> numWords = num.exportBits();

int_limited<w_count*32> lowNum;
int_limited<w_count*32> highNum;
lowNum.importBits(numWords, 0, w_count/2 + 1);
highNum.importBits(numWords, w_count/2 + 1, w_count);
```

#### Type casting

Implicit casting into `int_limited` is allowed from 32 and 64 bit standard library integers (`uint64_t`, `int64_t`, `unsigned int`, `int`),
from strings and char pointers and from other `int_limited` instances.

However conversion from `int_limited` to a standard type is strictly explicit, requiring explicit casting (e.g. `int zero = (int)num`).

Such explicit casting simply returns the bits in the least significant word and casts them to the respective type. \
For example:
```cpp
int_limited<256> num = -1;
uint64_t truncatedNum = (uint64_t)num;
// True, because of the two's complement representation of num
assert(truncatedNum == UINT64_MAX);
int_limited<512> a = num;
int_limited<512> b = -1;
int_limited<512> c = "-1";
// Both true
assert(a == b);
assert(b == c);
```

Explicit casting is allowed from `int_limited` to `uint64_t`, `int64_t`, `unsigned int`, `int` and `char`.

Note that since implicit conversion from `int_limited` to `bool` is not allowed, usage of a class instance by itself as a condition is not possible. The recommended method of checking if a number is non-zero is `example != 0` or `!!example` for possibly faster performance.


#### Conversion to std::string

Class instances can be converted to a string with the member function `toString(uint32_t base = 10)`. By default, it will convert the value into base 10, however an argument may be passed to convert to a different number base.
The size of the number base is limited to the maximum value of an unsigned 32 bit integer.

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

Multiplication uses a quadratic algorithm while one of the values is less than 512 bits (subject to change).
Then Karatsuba's algorithm is used, with an asymptotic time complexity of $O(N^{\log_{2}3}) \approx O(N^{1.585})$, this however comes with the cost of $O(N\log(N))$ space complexity with a constant of $\approx 10$, which may cause an unexpected slow down for larger numbers.

Division and modulo both use a smart quadratic algorithm that doesn't grow as fast as multiplication, however it is slower until around 1000 bits.
For the full performance comparison, see the [measured benchmarks](./testing-int_limited/benchmark-results.md).

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

### Mathematical methods

Currently only three simple functions are supported.
All of them are methods applied to the class instance, returning a new value to work with, without changing the original.

- `int ilog2()` returns the integer binary log of the value, which is equivalent to the index of the largest $1$ in binary representation, or the number of bits required to store the value.
Because of these simplifications, its speed is a relatively fast $O(\log N)$
- `int_limited pow(uint32_t exp)` returns $value^exp$. It utilises fast exponentiation, however because a modulo argument isn't supported, values larger than the bit size will lose precision.
- `int_limited isqrt()` returns $floor(\sqrt{value})$. It runs up to $O(\log N)$ multiplications.

### Static functions

#### className()

Returns the namespace and the class name as a `std::string`.
For this class, it will be `"largeNumberLibrary::int_limited<bitSize>"`, where `bitSize` represents the chosen template's bit size.

# Benchmarks

[For int128](./testing-int128/benchmark-results.md).
[For int_limited (and Boost)](./testing-int_limited/benchmark-results.md).