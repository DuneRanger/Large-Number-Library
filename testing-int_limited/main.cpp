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

using namespace customBigInt;

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

std::vector<uint64_t> genMultipleUint64(uint64_t seed, int count) {
	std::vector<uint64_t> output(count, 0);
	for (int i = 0; i < count; i++) {
		uint64_t num = rnd64(seed);
		if ((seed + num) % 8 < 5) {
			num <<= (seed + num)%8;
		}
		if ((seed + num) % 7 < 3) {
			num <<= 16;
		}
		if ((seed + num) % 6 > 3) {
			num <<= 32;
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
		int curNumWords = bitSize/64 + (bitSize%64 > 0);
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
		std::vector<uint64_t> words = genMultipleUint64(randState, curNumWords);
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
		int curNumWords = bitSize/64 + (bitSize%64 > 0);
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
		std::vector<uint64_t> words = genMultipleUint64(randState, curNumWords);

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
	// std::cout << std::bitset<64>((uint64_t)a) << " " << std::bitset<64>((int64_t)b) << std::endl;
	if (a < 0) {
		a = ~a + 1;
		b *= -1;
	}
	// std::cout << std::bitset<64>((uint64_t)a) << " " << std::bitset<64>((int64_t)b) << std::endl;
	int rem = bitSize%64;
	int wordCount = bitSize / 64 + (rem > 0);
	for (int i = 0; i < wordCount; i++) {
		uint64_t boostTrunc = (uint64_t)(b & UINT64_MAX);
		if (i == wordCount -1 && rem > 0) {
			boostTrunc &= UINT64_MAX >> (64 - rem);
		}
		if ((uint64_t)a != boostTrunc) return false;
		a >>= 64;
		b >>= 64;
	}
	return true;
}

	char sign = ' ';
		words.push_back((uint64_t)a);
		a >>= 64;
template <int bitSize>
bool verifyCorrectnessOfInt_limited(int testNumberCount = 1000, uint64_t randState = 1) {
	std::cout << "====================================================================================================" << std::endl;
	std::cout << "VERIFYING ARITHMETIC CORRECTNESS OF " << int_limited<bitSize>::className() << " WITH BOOST cpp_int" << std::endl;
	std::cout << "====================================================================================================" << std::endl;

	std::cout << "GENERATING TEST NUMBERS..." << std::endl;

    std::vector<boostInt> testNumbersBoost = generateBoostIntegers(testNumberCount, bitSize, randState);
	boostInt bitLimiter = 1;
	bitLimiter <<= bitSize;

	std::vector<int_limited<bitSize>> testNumbersInt_limited = generateInt_limited<bitSize>(testNumberCount, randState);

	// for (int i = 0; i < 50; i++) {
	// 	std::cout << i << " boostInt : " << (int64_t)testNumbersBoost[i] << std::endl;
	// 	std::cout << i << " int_limi : " << (int)testNumbersInt_limited[i] << std::endl;
	// 	std::cout << int_limitedEqualBoost(testNumbersInt_limited[i], testNumbersBoost[i]) << std::endl;
	// }
	
	std::cout << "------------------" << std::endl;
	std::cout << "VERIFYING ADDITION" << std::endl;
	for (int i = 0; i < testNumberCount; i++) {
		for (int j = 0; j < testNumberCount; j++) {
			boostInt boostResult = testNumbersBoost[i] + testNumbersBoost[j];
			boostResult %= bitLimiter;
			int_limited<bitSize> myResult = testNumbersInt_limited[i] + testNumbersInt_limited[j];
			if (!int_limitedEqualBoost<bitSize>(myResult, boostResult)) {
				std::cout << "FAILED: " << testNumbersBoost[i] << " + " << testNumbersBoost[j] << std::endl;
				std::cout << boostResult << " " << (uint64_t)(myResult >> 64) << " " << (uint64_t)myResult << std::endl;
				return false;
			}
		}
	}
	std::cout << "PASSED ADDITION" << std::endl;
	std::cout << "------------------" << std::endl;

	std::cout << "VERIFYING SUBTRACTION" << std::endl;
	for (int i = 0; i < testNumberCount; i++) {
		for (int j = 0; j < testNumberCount; j++) {
			boostInt boostResult = testNumbersBoost[i] - testNumbersBoost[j];
			int_limited<bitSize> myResult = testNumbersInt_limited[i] - testNumbersInt_limited[j];
			if (!int_limitedEqualBoost<bitSize>(myResult, boostResult)) {
				std::cout << "FAILED: " << testNumbersBoost[i] << " - " << testNumbersBoost[j] << std::endl;
				return false;
			}
		}
	}
	std::cout << "PASSED SUBTRACTION" << std::endl;
	std::cout << "------------------" << std::endl;

	std::cout << "VERIFYING MULTIPLICATION" << std::endl;
	for (int i = 0; i < testNumberCount; i++) {
		for (int j = 0; j < testNumberCount; j++) {
			boostInt boostResult = testNumbersBoost[i] * testNumbersBoost[j];
			int_limited<bitSize> myResult = testNumbersInt_limited[i] * testNumbersInt_limited[j];
			if (!int_limitedEqualBoost<bitSize>(myResult, boostResult)) {
				std::cout << "FAILED: " << testNumbersBoost[i] << " * " << testNumbersBoost[j] << std::endl;
				return false;
			}
		}
	}
	std::cout << "PASSED MULTIPLICATION" << std::endl;
	std::cout << "------------------" << std::endl;
	
	std::cout << "VERIFYING DIVISION" << std::endl;
	for (int i = 0; i < testNumberCount; i++) {
		for (int j = 0; j < testNumberCount; j++) {
			if (testNumbersBoost[j] == 0) continue;
			boostInt boostResult = testNumbersBoost[i] / testNumbersBoost[j];
			int_limited<bitSize> myResult = testNumbersInt_limited[i] / testNumbersInt_limited[j];
			if (!int_limitedEqualBoost<bitSize>(myResult, boostResult)) {
				std::cout << "FAILED: " << testNumbersBoost[i] << " / " << testNumbersBoost[j] << std::endl;
				return false;
			}
		}
	}
	std::cout << "PASSED DIVISION" << std::endl;
	std::cout << "------------------" << std::endl;

	std::cout << "VERIFYING MODULO" << std::endl;
	for (int i = 0; i < testNumberCount; i++) {
		for (int j = 0; j < testNumberCount; j++) {
			if (testNumbersBoost[j] == 0) continue;
			boostInt boostResult = testNumbersBoost[i] % testNumbersBoost[j];
			int_limited<bitSize> myResult = testNumbersInt_limited[i] % testNumbersInt_limited[j];
			if (!int_limitedEqualBoost<bitSize>(myResult, boostResult)) {
				std::cout << "FAILED: " << testNumbersBoost[i] << " % " << testNumbersBoost[j] << std::endl;
				return false;
			}
		}
	}
	std::cout << "PASSED MODULO" << std::endl;
	std::cout << "------------------" << std::endl;

	return true;
}

int main() {
	int testCaseAmount = 100;
	uint64_t randState = 1;
	verifyCorrectnessOfInt_limited<64>(testCaseAmount, randState);

	return 0;
}