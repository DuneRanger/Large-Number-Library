#include <cstdint>
#include <iostream>
#include <cstdio>
#include <string>
#include <cmath>
#include <vector>
#include <chrono>
#include <boost/multiprecision/cpp_int.hpp>
#include "../int_limited.hpp"

typedef boost::multiprecision::cpp_int boostInt;
typedef customBigInt::int_limited int_limited;

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
		int curNumWords = bitSize/64;
		if (randState % 8 < 5) {
			curNumWords -= randState%8;
		}
		if (randState % 7 < 3) {
			curNumWords /= 2;
		}
		if (randState % 6 > 3) {
			curNumWords /= 3;
		}
		std::vector<uint64_t> words = genMultipleUint64(randState, curNumWords);
		std::reverse(words.begin(), words.end());
		words[0] &= UINT64_MAX >> 1;

		boostInt boostNum;
		boost::multiprecision::import_bits(boostNum, words.begin(), words.end());
		if (randState%3 == 0) {
			boostNum *= -1;
		}
		values.push_back(boostNum);
	}
	return values;
}

// Reminder: When changing the constants here, change them in generateBoostIntegers as well
std::vector<int_limited> generateInt_limited(int count = 1000, int bitSize = 1024, uint64_t randState = 1) {
	std::vector<int_limited> values = {0, 1, -1, 2, -2, 17, -17};

	for (int i = values.size(); i < count; i++) {
		int curNumWords = bitSize/64;
		if (randState % 8 < 5) {
			curNumWords -= randState%8;
		}
		if (randState % 7 < 3) {
			curNumWords /= 2;
		}
		if (randState % 6 > 3) {
			curNumWords /= 3;
		}
		std::vector<uint64_t> words = genMultipleUint64(randState, curNumWords);
		words[curNumWords-1] &= UINT64_MAX >> 1;

		int_limited<bitSize> curValue;
		
		if (randState%3 == 0) {
			curValue = ~curValue + 1;
		}
		values.push_back(curValue);
	}
	return values;
}