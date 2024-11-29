#include <cstdint>
#include <iostream>
#include <string>
#include <cmath>
#include <vector>
#include <chrono>
#include <random>
#include <boost/multiprecision/cpp_int.hpp>
#include <boost/multiprecision/integer.hpp>
#include "../bigInt.hpp"
#include "../bigIntTest.hpp"
#include "../bigIntKaratsuba.hpp"

typedef boost::multiprecision::int128_t boostInt128;
typedef customBigInt::int128 baseInt128;
typedef customBigIntTest::int128 testInt128;
typedef customBigIntKaratsuba::int128 kInt128;


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

std::vector<uint64_t> genTwoUint64(uint64_t n) {
	uint64_t a = 0;
	uint64_t b = rnd64(n);
	if (n%4 == 1) {
		b >>= 32;
	}
	if (n%8 >= 2) {
		a = rnd64(n+1);
	}
	if (n%8 >= 6) {
		a >>= 32;
	}
	if (n%8 >= 4) {
		a |= customBigInt::BIT64_ON;
	}
	return {a, b};
}

// If comparing with a boost type, put it as the first argument
template <typename T1, typename T2>
bool twoInt128TypesEqual(T1 a, T2 b) {
	// because boost doesn't want to cooperate when casting into uint64_t, or even int64_t
	if (a < 0)  {
		a *= -1;
		b *= -1;
	}
	uint64_t A0 = static_cast<uint64_t>(a);
	uint64_t A1 = static_cast<uint64_t>(a >> 64);

	uint64_t B0 = static_cast<uint64_t>(b);
	uint64_t B1 = static_cast<uint64_t>(b >> 64);

	return A0 == B0 && A1 == B1;
}

template <typename T>
void printInt128Words(T a) {
	std::vector<uint64_t> words;
	int sign = 1;
	// Read DISCLAIMER ABOUT BOOST INTEGERS for clarification
	if (a < 0) {
		sign = -1;
		a *= -1;
	}
	do {
		words.push_back((uint64_t)a);
		a >>= 64;
	} while (a != 0);
	std::cout << sign << " * ";
	for (int i = words.size()-1; i > -1; i--) {
		std::cout << words[i] << " ";
	}
	std::cout << std::endl;
}

template <typename myInt128>
bool verifyCorrectnessOfMyInt128(int testNumberCount = 100) {
	std::cout << "====================================================================================================" << std::endl;
	std::cout << "VERIFYING ARITHMETIC CORRECTNESS OF " << myInt128::className() << " WITH BOOST int128_t" << std::endl;
	std::cout << "====================================================================================================" << std::endl;

	std::cout << "GENERATING TEST NUMBERS..." << std::endl;

	// These two vectors should have the same elements
    std::vector<boostInt128> testNumbersBoost = {0, -1, 1};
	std::vector<myInt128> testNumbersMyInt = {0, -1, 1};

	/*
	DISCLAIMER ABOUT BOOST INTEGERS
	boostInt128 uses its own variable for signed magnitude and can represent values from 2^(128)-1 to -(2^(128)-1)
	Additionally, it saves negative values int two's complement, if you imagine the signed magnitude as a hidden 129th bit
	This also means that when bitshifting a negative value, it keeps the sign (so -1 >> 10 == -1) by filling the shifted area with 1's
	
	However, despite bitshifting not changing the sign, any other bit operation does, for reasons unknown to me (or the documentation)
	Both & and | always change the sign to positive, only (~) uniquely can affect the sign bit and make it negative or positive (as one would expect)
	*/
	boostInt128 boostInt128Mask = (boostInt128(UINT64_MAX) << 64) + UINT64_MAX;
	uint64_t randState = 1;
    for (int i = testNumbersBoost.size(); i < testNumberCount; i++) {
        std::vector<uint64_t> words = genTwoUint64(randState);
		randState += 2;
        testNumbersMyInt.push_back(myInt128(words[0], words[1]));

		boostInt128 boostNum;
		boost::multiprecision::import_bits(boostNum, words.begin(), words.end());

		if (words[0] & customBigInt::BIT64_ON) {
			boostNum ^= boostInt128Mask;
			boostNum += 1;
			boostNum *= -1;
		}
        testNumbersBoost.push_back(boostNum);
    }

	// Yeah, nah, I'm not doing this after finding out how boost bitshifts, I'll just hope I got bitshifting bug-free

	// std::cout << "VERIFYING BITSHIFTING (PREREQUISITE)" << std::endl;
	// for (int i = 0; i < testNumberCount; i++) {
	// 	for (int j = 0; j < 150; j++) {
	// 		boostInt128 boostResult = testNumbersBoost[i] << j;
	// 		myInt128 myResult = testNumbersMyInt[i] << j;
	// 		if (!twoInt128TypesEqual<boostInt128, myInt128>(boostResult, myResult)) {
	// 			std::cout << "FAILED: " << testNumbersBoost[i] << " << " << j << std::endl;
	// 			printInt128Words<boostInt128>(boostResult);
	// 			printInt128Words<myInt128>(myResult);
	// 			return false;
	// 		}
	// 		// if (testNumbersBoost[i] < 0) boo
	// 		// boostResult = (testNumbersBoost[i] >> j) ^ ();
	// 		myResult = testNumbersMyInt[i] >> j;
	// 		if (!twoInt128TypesEqual<boostInt128, myInt128>(boostResult, myResult)) {
	// 			std::cout << "FAILED: " << testNumbersBoost[i] << " >> " << j << std::endl;
	// 			printInt128Words<boostInt128>(boostResult);
	// 			printInt128Words<myInt128>(myResult);
	// 			return false;
	// 		}
	// 	}
	// }
	// std::cout << "PASSED BITSHIFTING" << std::endl;
	std::cout << "------------------" << std::endl;

	std::cout << "VERIFYING ADDITION" << std::endl;
	for (int i = 0; i < testNumberCount; i++) {
		for (int j = 0; j < testNumberCount; j++) {
			boostInt128 boostResult = testNumbersBoost[i] + testNumbersBoost[j];
			myInt128 myResult = testNumbersMyInt[i] + testNumbersMyInt[j];
			if (!twoInt128TypesEqual<boostInt128, myInt128>(boostResult, myResult)) {
				std::cout << "FAILED: " << testNumbersBoost[i] << " + " << testNumbersBoost[j] << std::endl;
				printInt128Words<boostInt128>(boostResult);
				printInt128Words<myInt128>(myResult);
				return false;
			}
		}
	}
	std::cout << "PASSED ADDITION" << std::endl;
	std::cout << "------------------" << std::endl;

	std::cout << "VERIFYING SUBTRACTION" << std::endl;
	for (int i = 0; i < testNumberCount; i++) {
		for (int j = 0; j < testNumberCount; j++) {
			boostInt128 boostResult = testNumbersBoost[i] - testNumbersBoost[j];
			myInt128 myResult = testNumbersMyInt[i] - testNumbersMyInt[j];
			if (!twoInt128TypesEqual<boostInt128, myInt128>(boostResult, myResult)) {
				std::cout << "FAILED: " << testNumbersBoost[i] << " - " << testNumbersBoost[j] << std::endl;
				printInt128Words<boostInt128>(boostResult);
				printInt128Words<myInt128>(myResult);
				return false;
			}
		}
	}
	std::cout << "PASSED SUBTRACTION" << std::endl;
	std::cout << "------------------" << std::endl;

	std::cout << "VERIFYING MULTIPLICATION" << std::endl;
	for (int i = 0; i < testNumberCount; i++) {
		for (int j = 0; j < testNumberCount; j++) {
			boostInt128 boostResult = testNumbersBoost[i] * testNumbersBoost[j];
			myInt128 myResult = testNumbersMyInt[i] * testNumbersMyInt[j];
			if (!twoInt128TypesEqual<boostInt128, myInt128>(boostResult, myResult)) {
				std::cout << "FAILED: " << testNumbersBoost[i] << " * " << testNumbersBoost[j] << std::endl;
				printInt128Words<boostInt128>(boostResult);
				printInt128Words<myInt128>(myResult);
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
			boostInt128 boostResult = testNumbersBoost[i] / testNumbersBoost[j];
			myInt128 myResult = testNumbersMyInt[i] / testNumbersMyInt[j];
			if (!twoInt128TypesEqual<boostInt128, myInt128>(boostResult, myResult)) {
				std::cout << "FAILED: " << testNumbersBoost[i] << " / " << testNumbersBoost[j] << std::endl;
				printInt128Words<boostInt128>(boostResult);
				printInt128Words<myInt128>(myResult);
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
			boostInt128 boostResult = testNumbersBoost[i] % testNumbersBoost[j];
			myInt128 myResult = testNumbersMyInt[i] % testNumbersMyInt[j];
			if (!twoInt128TypesEqual<boostInt128, myInt128>(boostResult, myResult)) {
				std::cout << "FAILED: " << testNumbersBoost[i] << " % " << testNumbersBoost[j] << std::endl;
				printInt128Words<boostInt128>(boostResult);
				printInt128Words<myInt128>(myResult);
				return false;
			}
		}
	}
	std::cout << "PASSED MODULO" << std::endl;
	std::cout << "------------------" << std::endl;

	return true;
}

int main() {
	verifyCorrectnessOfMyInt128<baseInt128>(1000);
	verifyCorrectnessOfMyInt128<testInt128>();
	// verifyCorrectnessOfMyInt128<kInt128>();
	return 0;
} /*
	auto start = std::chrono::steady_clock::now();
	for (int i = min; i < max; i++) {
		for (int j = min; j < max; j++) {
			int64_t x = g + j;
		}
	}
	auto end = std::chrono::steady_clock::now();
	auto elapsed = std::chrono::duration<double>(end-start);
	std::cout << abs((max-min)*(max-min)) << " int64_t additions: \t\t" << elapsed.count() << " seconds" << std::endl;

	start = std::chrono::steady_clock::now();
	for (int i = min; i < max; i++) {
		for (int j = min; j < max; j++) {
			int128 x = f + j;
		}
	}
	end = std::chrono::steady_clock::now();
	elapsed = std::chrono::duration<double>(end-start);
	std::cout << abs((max-min)*(max-min)) << " int128 additions: \t\t" << elapsed.count() << " seconds" << std::endl;

	start = std::chrono::steady_clock::now();
	for (int i = min; i < max; i++) {
		for (int j = min; j < max; j++) {
			int64_t x = g - j;
		}
	}
	end = std::chrono::steady_clock::now();
	elapsed = std::chrono::duration<double>(end-start);
	std::cout << abs((max-min)*(max-min)) << " int64_t subtractions: \t" << elapsed.count() << " seconds" << std::endl;

	start = std::chrono::steady_clock::now();
	for (int i = min; i < max; i++) {
		for (int j = min; j < max; j++) {
			int128 x = f - j;
		}
	}
	end = std::chrono::steady_clock::now();
	elapsed = std::chrono::duration<double>(end-start);
	std::cout << abs((max-min)*(max-min)) << " int128 subtractions: \t\t" << elapsed.count() << " seconds" << std::endl;

	start = std::chrono::steady_clock::now();
	for (int i = min; i < max; i++) {
		for (int j = min; j < max; j++) {
			int64_t x = g * j;
		}
	}
	end = std::chrono::steady_clock::now();
	elapsed = std::chrono::duration<double>(end-start);
	std::cout << abs((max-min)*(max-min)) << " int64_t multiplications: \t" << elapsed.count() << " seconds" << std::endl;

	start = std::chrono::steady_clock::now();
	for (int i = min; i < max; i++) {
		for (int j = min; j < max; j++) {
			int128 x = f * j;
		}
	}
	end = std::chrono::steady_clock::now();
	elapsed = std::chrono::duration<double>(end-start);
	std::cout << abs((max-min)*(max-min)) << " int128 multiplications: \t" << elapsed.count() << " seconds" << std::endl;

	start = std::chrono::steady_clock::now();
	for (int i = min; i < max; i++) {
		for (int j = min; j < max; j++) {
			if (j == 0) continue;
			int64_t x = g / j;
		}
	}
	end = std::chrono::steady_clock::now();
	elapsed = std::chrono::duration<double>(end-start);
	std::cout << "~" << abs((max-min)*(max-min)) << " int64_t divisions: \t\t" << elapsed.count() << " seconds" << std::endl;

	start = std::chrono::steady_clock::now();
	for (int i = min; i < max; i++) {
		for (int j = min; j < max; j++) {
			if (j == 0) continue;
			int128 x = f / j;
		}
	}
	end = std::chrono::steady_clock::now();
	elapsed = std::chrono::duration<double>(end-start);
	std::cout << "~" << abs((max-min)*(max-min)) << " int128 divisions: \t\t" << elapsed.count() << " seconds" << std::endl;

	start = std::chrono::steady_clock::now();
	for (int i = min; i < max; i++) {
		for (int j = min; j < max; j++) {
			if (j == 0) continue;
			int64_t x = g % j;
		}
	}
	end = std::chrono::steady_clock::now();
	elapsed = std::chrono::duration<double>(end-start);
	std::cout << "~" << abs((max-min)*(max-min)) << " int64_t modulo: \t\t" << elapsed.count() << " seconds" << std::endl;

	start = std::chrono::steady_clock::now();
	for (int i = min; i < max; i++) {
		for (int j = min; j < max; j++) {
			if (j == 0) continue;
			int128 x = f % j;
		}
	}
	end = std::chrono::steady_clock::now();
	elapsed = std::chrono::duration<double>(end-start);
	std::cout << "~" << abs((max-min)*(max-min)) << " int128 modulo: \t\t" << elapsed.count() << " seconds" << std::endl;

	// return false;



} */