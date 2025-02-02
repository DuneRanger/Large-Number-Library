#include <cstdint>
#include <iostream>
#include <cstdio>
#include <string>
#include <cmath>
#include <vector>
#include <chrono>
#include <boost/multiprecision/cpp_int.hpp>
#include "../int128.hpp"

typedef boost::multiprecision::int128_t boostInt128;
typedef customBigInt::int128 int128;

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
	uint64_t b = 0;
	if (n%8 < 6) {
		b = rnd64(n);
	}
	if (n%8 < 2) {
		b >>= 32;
	}
	if (n%8 >= 4) {
		a = rnd64(n+1);
	}
	if (n%8 >= 6) {
		a >>= 32;
	}
	if (n%7 == 0) {
		a ^= UINT64_MAX;
	}
	return {a, b};
}

/*
	DISCLAIMER ABOUT BOOST INTEGERS
	boostInt128 uses its own variable for signed magnitude and can represent values from 2^(128)-1 to -(2^(128)-1)
	Additionally, it saves negative values int two's complement, if you imagine the signed magnitude as a hidden 129th bit
	This also means that when bitshifting a negative value, it keeps the sign (so -1 >> 10 == -1) by filling the shifted area with 1's

	Reminder: When changing the constants here, change them in generateMyInt128Numbers as well
*/
std::vector<boostInt128> generateBoost128Numbers(int count = 1000, uint64_t randState = 1) {
	std::vector<boostInt128> boostNumbers = {0, 1, -1};

	boostInt128 boostInt128Mask = (boostInt128(UINT64_MAX) << 64) + UINT64_MAX;
    for (int i = boostNumbers.size(); i < count; i++) {
        std::vector<uint64_t> words = genTwoUint64(randState);
		randState += 2;

		boostInt128 boostNum;
		boost::multiprecision::import_bits(boostNum, words.begin(), words.end());

		if (words[0] & customBigInt::BIT64_ON) {
			boostNum ^= boostInt128Mask;
			boostNum += 1;
			boostNum *= -1;
		}
        boostNumbers.push_back(boostNum);
    }
	return boostNumbers;
}

// Reminder: When changing the constants here, change them in generateBoost128Numbers as well
template <typename myInt128	>
std::vector<myInt128> generateMyInt128Numbers(int count = 1000, uint64_t randState = 1) {
	std::vector<myInt128> myIntNumbers = {0, 1, -1};

    for (int i = myIntNumbers.size(); i < count; i++) {
        std::vector<uint64_t> words = genTwoUint64(randState);
		randState += 2;
        myIntNumbers.push_back(myInt128(words[0], words[1]));
    }
	return myIntNumbers;
} 

template <typename T1, typename T2>
bool twoInt128TypesEqual(T1 a, T2 b) {
	// Note that specifically setting one of the variables as one for testing is recommended instead of this
	// Because that allows the use of getting the two's complement through bit NOT + 1
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
	char sign = ' ';
	// Because of boost integers and their casting
	if (a < 0) {
		sign = '-';
		a *= -1;
	}
	do {
		words.push_back((uint64_t)a);
		a >>= 64;
	} while (a != 0);
	std::cout << sign << "1 * ";
	for (int i = words.size()-1; i > -1; i--) {
		std::cout << words[i] << " ";
	}
	std::cout << std::endl;
}

template <typename myInt128>
bool verifyCorrectnessOfMyInt128(int testNumberCount = 1000, uint64_t randState = 1) {
	std::cout << "====================================================================================================" << std::endl;
	std::cout << "VERIFYING ARITHMETIC CORRECTNESS OF " << myInt128::className() << " WITH BOOST int128_t" << std::endl;
	std::cout << "====================================================================================================" << std::endl;

	std::cout << "GENERATING TEST NUMBERS..." << std::endl;

    std::vector<boostInt128> testNumbersBoost = generateBoost128Numbers(testNumberCount, randState);
	std::vector<myInt128> testNumbersMyInt = generateMyInt128Numbers<myInt128>(testNumberCount, randState);
	
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


std::vector<double> speedBenchmarkBoost(int testNumberCount = 3000, uint64_t randState = 1) {
	std::cout << std::endl;
	// std::cout << "====================================================================================================" << std::endl;
	std::cout << "BENCHMARKING Boost Int128 ON " << testNumberCount*testNumberCount << " CASES" << std::endl;
	// std::cout << "====================================================================================================" << std::endl;

	std::cout << "GENERATING TEST NUMBERS..." << std::endl;

	std::vector<boostInt128> testNumbersBoost = generateBoost128Numbers(testNumberCount, randState);
	std::vector<double> results;
	boostInt128 boostResult = 0;

	// I'm not sure how to improve the repetitiveness of these for loops without sacrificing some kind of time inaccuracy (a function with a switch statement)
	// Printing out boostResult at the end seems to ward off O1 optimizing away the operations
	// The empty loop only plays a role for compilation with O0
	std::cout << "MEASURING EMPTY FOR LOOP OFFSET: ";
	auto start_time = std::chrono::steady_clock::now();
	for (int i = 0; i < testNumberCount; i++) {
		for (int j = 0; j < testNumberCount; j++) {

		}
	}
	auto end_time = std::chrono::steady_clock::now();
	std::chrono::duration<double> emptyLoop(end_time - start_time);
	std::cout << emptyLoop.count() << " seconds" << std::endl;

	std::cout << "MEASURING ADDITION: ";
	start_time = std::chrono::steady_clock::now();
	for (int i = 0; i < testNumberCount; i++) {
		for (int j = 0; j < testNumberCount; j++) {
			boostResult = testNumbersBoost[i] + testNumbersBoost[j];
		}
	}
	end_time = std::chrono::steady_clock::now();
	std::chrono::duration<double> duration(end_time - start_time);
	std::cout << duration.count() - emptyLoop.count() << " seconds" << std::endl;
	results.push_back(duration.count() - emptyLoop.count());

	std::cout << "MEASURING SUBSTRACTION: ";
	start_time = std::chrono::steady_clock::now();
	for (int i = 0; i < testNumberCount; i++) {
		for (int j = 0; j < testNumberCount; j++) {
			boostResult = testNumbersBoost[i] - testNumbersBoost[j];
		}
	}
	end_time = std::chrono::steady_clock::now();
	duration = (end_time - start_time);
	std::cout << duration.count() - emptyLoop.count() << " seconds" << std::endl;
	results.push_back(duration.count() - emptyLoop.count());

	std::cout << "MEASURING MULTIPLICATION: ";
	start_time = std::chrono::steady_clock::now();
	for (int i = 0; i < testNumberCount; i++) {
		for (int j = 0; j < testNumberCount; j++) {
			boostResult = testNumbersBoost[i] * testNumbersBoost[j];
		}
	}
	end_time = std::chrono::steady_clock::now();
	duration = (end_time - start_time);
	std::cout << duration.count() - emptyLoop.count() << " seconds" << std::endl;
	results.push_back(duration.count() - emptyLoop.count());
	
	std::cout << "MEASURING DIVISION: ";
	start_time = std::chrono::steady_clock::now();
	for (int i = 0; i < testNumberCount; i++) {
		for (int j = 0; j < testNumberCount; j++) {
			if (testNumbersBoost[j] == 0) continue;
			boostResult = testNumbersBoost[i] / testNumbersBoost[j];
		}
	}
	end_time = std::chrono::steady_clock::now();
	duration = (end_time - start_time);
	std::cout << duration.count() - emptyLoop.count() << " seconds" << std::endl;
	results.push_back(duration.count() - emptyLoop.count());

	std::cout << "MEASURING MODULO: ";
	start_time = std::chrono::steady_clock::now();
	for (int i = 0; i < testNumberCount; i++) {
		for (int j = 0; j < testNumberCount; j++) {
			if (testNumbersBoost[j] == 0) continue;
			boostResult = testNumbersBoost[i] % testNumbersBoost[j];
		}
	}
	end_time = std::chrono::steady_clock::now();
	duration = (end_time - start_time);
	std::cout << duration.count() - emptyLoop.count() << " seconds" << std::endl;
	results.push_back(duration.count() - emptyLoop.count());

	// printing boostResult to ward away optimization, but also putting it in an if statement to not bloat the log
	if (boostResult == -2) {
		std::cout << boostResult << std::endl;
	}

	return results;
}

template <typename myInt128>
std::vector<double> speedBenchmarkMyInt(int testNumberCount = 3000, uint64_t randState = 1) {
	// std::cout << "====================================================================================================" << std::endl;
	std::cout << "BENCHMARKING " << myInt128::className() << " ON " << testNumberCount*testNumberCount << " CASES" << std::endl;
	// std::cout << "====================================================================================================" << std::endl;

	std::cout << "GENERATING TEST NUMBERS..." << std::endl;

	std::vector<myInt128> testNumbersMyInt128 = generateMyInt128Numbers<myInt128>(testNumberCount, randState);
	std::vector<double> results;
	myInt128 myResult = 0;
	
	// I'm not sure how to improve the repetitiveness of these for loops without sacrificing some kind of time inaccuracy (a function with a switch statement)
	// Printing out boostResult at the end seems to ward off optimizing away the operations
	// The empty loop only plays a significant role for compilation with O0 optimization
	std::cout << "MEASURING EMPTY FOR LOOP OFFSET: ";
	auto start_time = std::chrono::steady_clock::now();
	for (int i = 0; i < testNumberCount; i++) {
		for (int j = 0; j < testNumberCount; j++) {

		}
	}
	auto end_time = std::chrono::steady_clock::now();
	std::chrono::duration<double> emptyLoop(end_time - start_time);
	std::cout << emptyLoop.count() << " seconds" << std::endl;

	std::cout << "MEASURING ADDITION: ";
	start_time = std::chrono::steady_clock::now();
	for (int i = 0; i < testNumberCount; i++) {
		for (int j = 0; j < testNumberCount; j++) {
			myResult = testNumbersMyInt128[i] + testNumbersMyInt128[j];
		}
	}
	end_time = std::chrono::steady_clock::now();
	std::chrono::duration<double> duration(end_time - start_time);
	std::cout << duration.count() - emptyLoop.count() << " seconds" << std::endl;
	results.push_back(duration.count() - emptyLoop.count());

	std::cout << "MEASURING SUBSTRACTION: ";
	start_time = std::chrono::steady_clock::now();
	for (int i = 0; i < testNumberCount; i++) {
		for (int j = 0; j < testNumberCount; j++) {
			myResult = testNumbersMyInt128[i] - testNumbersMyInt128[j];
		}
	}
	end_time = std::chrono::steady_clock::now();
	duration = (end_time - start_time);
	std::cout << duration.count() - emptyLoop.count() << " seconds" << std::endl;
	results.push_back(duration.count() - emptyLoop.count());

	std::cout << "MEASURING MULTIPLICATION: ";
	start_time = std::chrono::steady_clock::now();
	for (int i = 0; i < testNumberCount; i++) {
		for (int j = 0; j < testNumberCount; j++) {
			myResult = testNumbersMyInt128[i] * testNumbersMyInt128[j];
		}
	}
	end_time = std::chrono::steady_clock::now();
	duration = (end_time - start_time);
	std::cout << duration.count() - emptyLoop.count() << " seconds" << std::endl;
	results.push_back(duration.count() - emptyLoop.count());
	
	std::cout << "MEASURING DIVISION: ";
	start_time = std::chrono::steady_clock::now();
	for (int i = 0; i < testNumberCount; i++) {
		for (int j = 0; j < testNumberCount; j++) {
			if (testNumbersMyInt128[j] == 0) continue;
			myResult = testNumbersMyInt128[i] / testNumbersMyInt128[j];
		}
	}
	end_time = std::chrono::steady_clock::now();
	duration = (end_time - start_time);
	std::cout << duration.count() - emptyLoop.count() << " seconds" << std::endl;
	results.push_back(duration.count() - emptyLoop.count());

	std::cout << "MEASURING MODULO: ";
	start_time = std::chrono::steady_clock::now();
	for (int i = 0; i < testNumberCount; i++) {
		for (int j = 0; j < testNumberCount; j++) {
			if (testNumbersMyInt128[j] == 0) continue;
			myResult = testNumbersMyInt128[i] % testNumbersMyInt128[j];
		}
	}
	end_time = std::chrono::steady_clock::now();
	duration = (end_time - start_time);
	std::cout << duration.count() - emptyLoop.count() << " seconds" << std::endl;
	results.push_back(duration.count() - emptyLoop.count());

	if (myResult == -2) {
		std::cout << myResult << std::endl;
	}

	return results;
}

std::vector<double> averageBenchmarkBoost(int testNumberCount = 3000, int iterations = 5) {
	std::cout << "====================================================================================================" << std::endl;
	std::cout << "CONDUCTING " << iterations << " BENCHMARKS OF Boost Int128 ON " << testNumberCount*testNumberCount << " CASES" << std::endl;
	std::cout << "====================================================================================================" << std::endl;
	
	uint64_t randState = 1;
	// addition, subtraction, multiplication, division, modulo
	std::vector<double> operationTimes = {0, 0, 0, 0, 0};
	std::vector<std::string> operationNames = {"ADDITION", "SUBTRACTION", "MULTIPLICATION", "DIVISION", "MODULO"};

	for (int i = 0; i < iterations; i++) {
		std::cout << std::endl << "Iteration " << i+1 << ":" << std::endl;
		auto results = speedBenchmarkBoost(testNumberCount, randState);
		for (int j = 0; j < results.size(); j++) {
			operationTimes[j] += results[j];
		}
		randState += testNumberCount*testNumberCount + 1;
	}
	for (int i = 0; i < operationTimes.size(); i++) {
		operationTimes[i] /= iterations;
		std::cout << "AVERAGE" << operationNames[i] << "TIME: " << operationTimes[i] << std::endl;
	}
	return operationTimes;
}

template <typename myInt128>
std::vector<double> averageBenchmarkMyInt128(int testNumberCount = 3000, int iterations = 5) {
	std::cout << "====================================================================================================" << std::endl;
	std::cout << "CONDUCTING " << iterations << " BENCHMARKS OF " << myInt128::className() << " ON " << testNumberCount*testNumberCount << " CASES" << std::endl;
	std::cout << "====================================================================================================" << std::endl;
	
	uint64_t randState = 1;
	// addition, subtraction, multiplication, division, modulo
	std::vector<double> operationTimes = {0, 0, 0, 0, 0};
	std::vector<std::string> operationNames = {"ADDITION", "SUBTRACTION", "MULTIPLICATION", "DIVISION", "MODULO"};

	for (int i = 0; i < iterations; i++) {
		std::cout << std::endl << "Iteration " << i+1 << ":" << std::endl;
		auto results = speedBenchmarkMyInt<myInt128>(testNumberCount, randState);
		for (int j = 0; j < results.size(); j++) {
			operationTimes[j] += results[j];
		}
		randState += testNumberCount*testNumberCount + 1;
	}
	std::cout << std::endl;
	for (int i = 0; i < operationTimes.size(); i++) {
		operationTimes[i] /= iterations;
		std::cout << "AVERAGE " << operationNames[i] << " TIME: " << operationTimes[i] << std::endl;
	}
	return operationTimes;
}

void formatBenchmarkTimesTable(std::vector<std::vector<double>> averageBenchmarkResults) {
	std::vector<std::string> operationNames = {"ADDITION", "SUBTRACTION", "MULTIPLICATION", "DIVISION", "MODULO"};
	std::vector<std::string> benchmarkResultTypeNames = {"Boost int128", "int128"};
	assert(averageBenchmarkResults.size() == benchmarkResultTypeNames.size());
	assert(averageBenchmarkResults[0].size() == operationNames.size());

	printf("+");
	for (int buffer = 0; buffer < operationNames.size()+ 1; buffer++) {
		printf("----------------+"); // 16 long, because of the space before %-15
	}
	printf("\n");
	std::printf("| %-15s|", "");
	for (std::string opName : operationNames) {
		printf(" %-15s|", opName.c_str());
	}
	printf("\n+");
	for (int buffer = 0; buffer < operationNames.size()+ 1; buffer++) {
		printf("----------------+"); // 16 long, because of the space before %-15
	}
	printf("\n");
	for (int i = 0; i < averageBenchmarkResults.size(); i++) {
		printf("| %-15s|", benchmarkResultTypeNames[i].c_str());
		for (int j = 0; j < operationNames.size(); j++) {
			printf(" %-15.8f|", averageBenchmarkResults[i][j]);
		}
		printf("\n+");
		for (int buffer = 0; buffer < operationNames.size() + 1; buffer++) {
			printf("----------------+"); // 15 long
		}
		printf("\n");
	}
}

int main() {
	int testCaseAmount = 5000;
	int iterations = 10;
	uint64_t randState = 1;
	std::vector<std::vector<double>> averageBenchmarkResults;
	verifyCorrectnessOfMyInt128<int128>();
	// speedBenchmarkBoost(testCaseAmount);
	// speedBenchmarkMyInt<int128>(testCaseAmount);

	averageBenchmarkResults.push_back(averageBenchmarkBoost(testCaseAmount, iterations));
	averageBenchmarkResults.push_back(averageBenchmarkMyInt128<int128>(testCaseAmount, iterations));

	std::cout << std::endl;
	std::cout << "UNIQUE TEST NUMBERS PER BENCHMARK: " << testCaseAmount << " (INCLUDING 0, 1, -1)" << std::endl;
	std::cout << "TOTAL CALCULATIONS PER TEST: " << testCaseAmount*testCaseAmount << std::endl;
	std::cout << "TIMES AVERAGED OVER " << iterations << " DIFFERENT BENCHMARK ITERATIONS" << std::endl;
	std::cout << "INITIAL RANDSTATE: " << randState << std::endl; 

	formatBenchmarkTimesTable(averageBenchmarkResults);
	return 0;
}