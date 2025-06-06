#include <cstdint>
#include <iostream>
#include <cstdio>
#include <string>
#include <cmath>
#include <vector>
#include <chrono>
#include <array>
#include <boost/multiprecision/cpp_int.hpp>
#include "../int_limited.hpp"

typedef boost::multiprecision::cpp_int boostInt;

using namespace largeNumberLibrary;

// From https://www.reddit.com/r/C_Programming/comments/ozew2u/comment/h7zijm8
uint64_t rnd64(uint64_t n)
{
    const uint64_t z = 0x9FB21C651E98DF25;

    n ^= ((n << 49) | (n >> 15)) ^ ((n << 24) | (n >> 40));
    n *= z;
    n ^= n >> 35;
    n *= z;
    n ^= n >> 28;

    return n;
}

std::vector<uint32_t> genMultipleUint32(uint64_t seed, int count) {
	std::vector<uint32_t> output(count, 0);
	for (int i = 0; i < count; i++) {
		uint32_t num = uint32_t(rnd64(seed));
		if ((seed + num) % 8 < 5) {
			num <<= (seed + num)%8;
		}
		if ((seed + num) % 7 < 3) {
			num <<= 8;
		}
		if ((seed + num) % 6 > 3) {
			num <<= 16;
		}
		output[i] = num;
		seed++;
	}
	return output;
}

// Reminder: When changing the constants here, change them in generateInt_limited as well
std::vector<boostInt> generateBoostIntegers(int count = 1000, int bitSize = 1024, uint64_t randState = 1) {
	std::vector<boostInt> values = {0, 1, -1, 2, -2, 17, -17};
	boostInt bitLimiter = 1;
	bitLimiter <<= bitSize;

	for (int i = values.size(); i < count; i++) {
		int curNumWords = bitSize/32 + (bitSize%32 > 0);
		if (curNumWords > 1 && randState % 8 < 5) {
			curNumWords -= randState%8;
			curNumWords = std::max(curNumWords, 1);
		}
		if (curNumWords > 3 && randState % 7 < 3) {
			curNumWords /= 2;
		}
		if (curNumWords > 5 && randState % 6 > 3) {
			curNumWords /= 3;
		}
		std::vector<uint32_t> words = genMultipleUint32(randState, curNumWords);
		std::reverse(words.begin(), words.end());

		boostInt boostNum;
		boost::multiprecision::import_bits(boostNum, words.begin(), words.end());
		boostNum %= bitLimiter;
		boostNum >>= 1;
		if (randState%3 == 0) {
			boostNum *= -1;
		}
		values.push_back(boostNum);
		randState += curNumWords;
	}
	return values;
}

// Reminder: When changing the constants here, change them in generateBoostIntegers as well
template <int bitSize>
std::vector<int_limited<bitSize>> generateInt_limited(int count = 1000, uint64_t randState = 1) {
	std::vector<int_limited<bitSize>> values = {0, 1, -1, 2, -2, 17, -17};

	for (int i = values.size(); i < count; i++) {
		int curNumWords = bitSize/32 + (bitSize%32 > 0);
		if (curNumWords > 1 && randState % 8 < 5) {
			curNumWords -= randState%8;
			curNumWords = std::max(curNumWords, 1);
		}
		if (curNumWords > 3 && randState % 7 < 3) {
			curNumWords /= 2;
		}
		if (curNumWords > 5 && randState % 6 > 3) {
			curNumWords /= 3;
		}
		std::vector<uint32_t> words = genMultipleUint32(randState, curNumWords);

		int_limited<bitSize> curValue = 0;
		curValue.importBits(words);
		curValue >>= 1;
		if (randState%3 == 0) {
			curValue = ~curValue + 1;
		}
		values.push_back(curValue);
		randState += curNumWords;
	}
	return values;
}

template <int bitSize>
bool int_limitedEqualBoost(int_limited<bitSize> a, boostInt b) {
	if (a < 0) {
		a = ~a + 1;
		b *= -1;
	}
	int rem = bitSize%32;
	int wordCount = bitSize / 32 + (rem > 0);
	for (int i = 0; i < wordCount; i++) {
		uint32_t boostTrunc = (uint32_t)(b & UINT32_MAX);
		if (i == wordCount-1 && rem > 0) {
			boostTrunc &= UINT32_MAX >> (32 - rem);
		}
		if ((uint32_t)a != boostTrunc) return false;
		a >>= 32;
		b >>= 32;
	}
	return true;
}

template <typename T>
void printIntWords(T a, bool mult = true) {
	std::vector<uint32_t> words;
	char sign = ' ';
	// Because of boost integers and their casting
	if (a < 0) {
		sign = '-';
		if (mult) a *= -1;
		else a = ~a + 1;
	}
	do {
		words.push_back((uint32_t)a);
		a >>= 32;
	} while (a != 0);
	std::cout << sign << "1 * ";
	for (int i = words.size()-1; i > -1; i--) {
		std::cout << words[i] << " ";
	}
	std::cout << std::endl;
}

// Prints a vector of boostInt and int_limited<bitSize> alternating for easy comparison
// Has an option to either print each 32 bit word individually, or as a whole string in base 10 (default)
template <int bitSize>
void printGeneratedNumbers(std::vector<boostInt>& testNumbersBoost, std::vector<int_limited<bitSize>>& testNumbersInt_limited, bool printAsWords = false) {
	if (printAsWords) {
		for (int i = 0; i < testNumbersBoost.size(); i++) {
			std::cout << i << " boostInt : ";
			printIntWords<boostInt>(testNumbersBoost[i]);
			std::cout << i << " int_limi : ";
			printIntWords<int_limited<bitSize>>(testNumbersInt_limited[i], false);
			std::cout << "Bits equal?: " << int_limitedEqualBoost(testNumbersInt_limited[i], testNumbersBoost[i]) << std::endl;
		}
	}
	else {
		for (int i = 0; i < testNumbersBoost.size(); i++) {
			std::cout << i << " boostInt : " << testNumbersBoost[i] << std::endl;
			std::cout << i << " int_limi : " << testNumbersInt_limited[i] << std::endl;;
			std::cout << "Bits equal?: " << int_limitedEqualBoost(testNumbersInt_limited[i], testNumbersBoost[i]) << std::endl;
		}
	}
	return;
}

template <int bitSize>
bool verifyCorrectnessOfInt_limited(int testNumberCount = 1000, uint64_t randState = 1, bool printTestNumbers = false) {
	std::cout << "====================================================================================================" << std::endl;
	std::cout << "VERIFYING ARITHMETIC CORRECTNESS OF " << int_limited<bitSize>::className() << " WITH BOOST cpp_int" << std::endl;
	std::cout << "====================================================================================================" << std::endl;

	std::cout << "GENERATING TEST NUMBERS..." << std::endl;

    std::vector<boostInt> testNumbersBoost = generateBoostIntegers(testNumberCount, bitSize, randState);
	boostInt bitLimiter = 1;
	bitLimiter <<= bitSize;

	std::vector<int_limited<bitSize>> testNumbersInt_limited = generateInt_limited<bitSize>(testNumberCount, randState);

	if (testNumbersBoost.size() != testNumbersInt_limited.size()) {
		std::cout << "\033[1;31mERROR: testNumbersBoost.size() and testNumbersInt_limited.size() do not equal!\033[0m" << std::endl;
		return false;
	}

	if (printTestNumbers) printGeneratedNumbers<bitSize>(testNumbersBoost, testNumbersInt_limited, false);
	
	std::cout << "VERIFYING ADDITION: ";
	for (int i = 0; i < testNumberCount; i++) {
		for (int j = 0; j < testNumberCount; j++) {
			boostInt boostResult = testNumbersBoost[i] + testNumbersBoost[j];
			boostResult %= bitLimiter;
			int_limited<bitSize> myResult = testNumbersInt_limited[i] + testNumbersInt_limited[j];
			if (!int_limitedEqualBoost<bitSize>(myResult, boostResult)) {
				std::cout << "\033[1;31mFAILED: " << testNumbersBoost[i] << " + " << testNumbersBoost[j] << "\033[0m" << std::endl;
				return false;
			}
		}
	}
	std::cout << "\033[32mPASSED ADDITION\033[0m" << std::endl;

	std::cout << "VERIFYING SUBTRACTION: ";
	for (int i = 0; i < testNumberCount; i++) {
		for (int j = 0; j < testNumberCount; j++) {
			boostInt boostResult = testNumbersBoost[i] - testNumbersBoost[j];
			int_limited<bitSize> myResult = testNumbersInt_limited[i] - testNumbersInt_limited[j];
			if (!int_limitedEqualBoost<bitSize>(myResult, boostResult)) {
				std::cout << "\033[1;31mFAILED: " << testNumbersBoost[i] << " - " << testNumbersBoost[j] << "\033[0m" << std::endl;
				return false;
			}
		}
	}
	std::cout << "\033[32mPASSED SUBTRACTION\033[0m" << std::endl;

	std::cout << "VERIFYING MULTIPLICATION: ";
	for (int i = 0; i < testNumberCount; i++) {
		for (int j = 0; j < testNumberCount; j++) {
			boostInt boostResult = testNumbersBoost[i] * testNumbersBoost[j];
			int_limited<bitSize> myResult = testNumbersInt_limited[i] * testNumbersInt_limited[j];
			if (!int_limitedEqualBoost<bitSize>(myResult, boostResult)) {
				std::cout << "\033[1;31mFAILED: " << testNumbersBoost[i] << " * " << testNumbersBoost[j] << "\033[0m" << std::endl;
				return false;
			}
		}
	}
	std::cout << "\033[32mPASSED MULTIPLICATION\033[0m" << std::endl;
	
	std::cout << "VERIFYING DIVISION: ";
	for (int i = 0; i < testNumberCount; i++) {
		for (int j = 0; j < testNumberCount; j++) {
			if (testNumbersBoost[j] == 0) continue;
			boostInt boostResult = testNumbersBoost[i] / testNumbersBoost[j];
			int_limited<bitSize> myResult = testNumbersInt_limited[i] / testNumbersInt_limited[j];
			if (!int_limitedEqualBoost<bitSize>(myResult, boostResult)) {
				std::cout << "\033[1;31mFAILED: " << testNumbersBoost[i] << " / " << testNumbersBoost[j] << "\033[0m" << std::endl;
				printIntWords<boostInt>(boostResult);
				printIntWords<int_limited<bitSize>>(myResult, false);
				return false;
			}
		}
	}
	std::cout << "\033[32mPASSED DIVISION\033[0m" << std::endl;

	std::cout << "VERIFYING MODULO: ";
	for (int i = 0; i < testNumberCount; i++) {
		for (int j = 0; j < testNumberCount; j++) {
			if (testNumbersBoost[j] == 0) continue;
			boostInt boostResult = testNumbersBoost[i] % testNumbersBoost[j];
			int_limited<bitSize> myResult = testNumbersInt_limited[i] % testNumbersInt_limited[j];
			if (!int_limitedEqualBoost<bitSize>(myResult, boostResult)) {
				std::cout << "\033[1;31mFAILED: " << testNumbersBoost[i] << " % " << testNumbersBoost[j] << "\033[0m" << std::endl;
				printIntWords<boostInt>(boostResult);
				printIntWords<int_limited<bitSize>>(myResult, false);
				return false;
			}
		}
	}
	std::cout << "\033[32mPASSED MODULO\033[0m" << std::endl;

	return true;
}

int main() {
	int testCaseAmount = 1000;
	uint64_t randState = 1;


	// Tests multiples of 64 and different offsets
	// verifyCorrectnessOfInt_limited<16>(testCaseAmount, randState);
	// verifyCorrectnessOfInt_limited<17>(testCaseAmount, randState);
	// verifyCorrectnessOfInt_limited<27>(testCaseAmount, randState);
	// verifyCorrectnessOfInt_limited<32>(testCaseAmount, randState);
	// verifyCorrectnessOfInt_limited<32*2>(testCaseAmount, randState);
	// verifyCorrectnessOfInt_limited<32*2 + 1>(testCaseAmount, randState);
	// verifyCorrectnessOfInt_limited<32*4>(testCaseAmount, randState);
	verifyCorrectnessOfInt_limited<32*4 + 2>(testCaseAmount, randState); // bad modulo
	testCaseAmount = 500;
	// verifyCorrectnessOfInt_limited<32*6>(testCaseAmount, randState);
	verifyCorrectnessOfInt_limited<32*6 + 3>(testCaseAmount, randState); // bad modulo
	// verifyCorrectnessOfInt_limited<32*8>(testCaseAmount, randState);
	verifyCorrectnessOfInt_limited<32*8 + 6>(testCaseAmount, randState); // bad modulo
	testCaseAmount = 300;
	// verifyCorrectnessOfInt_limited<32*20>(testCaseAmount, randState);
	verifyCorrectnessOfInt_limited<32*20 + 17>(testCaseAmount, randState); // bad modulo
	// verifyCorrectnessOfInt_limited<32*32>(testCaseAmount, randState); // 1024
	// verifyCorrectnessOfInt_limited<32*32 + 31>(testCaseAmount, randState); // 1055
	testCaseAmount = 200;
	// verifyCorrectnessOfInt_limited<32*42>(testCaseAmount, randState);
	// verifyCorrectnessOfInt_limited<32*42 + 32>(testCaseAmount, randState);
	// verifyCorrectnessOfInt_limited<32*64>(testCaseAmount, randState); // 2048
	verifyCorrectnessOfInt_limited<32*64 + 59>(testCaseAmount, randState); // 2107 bad modulo
	testCaseAmount = 100;
	// verifyCorrectnessOfInt_limited<32*128>(testCaseAmount, randState); // 4096
	verifyCorrectnessOfInt_limited<32*128 + 63>(testCaseAmount, randState); // 4159 bad modulo

	testCaseAmount = 50;
	// verifyCorrectnessOfInt_limited<32*512>(testCaseAmount, randState); // 16384
	// verifyCorrectnessOfInt_limited<32*1024>(testCaseAmount, randState); // 32768
	// testCaseAmount = 20;
	// verifyCorrectnessOfInt_limited<32*2048>(testCaseAmount, randState); // 65536

	return 0;
}