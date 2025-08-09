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
std::vector<double> speedBenchmark_int_limited(int testNumberCount = 1000, uint64_t randState = 1) {
	std::cout << "====================================================================================================" << std::endl;
	std::cout << "Benchmarking " << int_limited<bitSize>::className() << " on " << uint64_t(testNumberCount)*testNumberCount << " cases" << std::endl;

	std::vector<int_limited<bitSize>> testNumbers_int_limited = generateInt_limited<bitSize>(testNumberCount, randState);
	std::vector<double> results;
	int_limited<bitSize> testResult = 0; // checked at the end to stop optimization from skipping loops
	
	std::cout << "Measuring Addition: ";
	auto start_time = std::chrono::steady_clock::now();
	for (int i = 0; i < testNumberCount; i++) {
		for (int j = 0; j < testNumberCount; j++) {
			testResult = testNumbers_int_limited[i] + testNumbers_int_limited[j];
		}
	}
	auto end_time = std::chrono::steady_clock::now();
	std::chrono::duration<double> duration(end_time - start_time);
	std::cout << duration.count() << " seconds" << std::endl;
	results.push_back(duration.count());

	std::cout << "Measuring Subtraction: ";
	start_time = std::chrono::steady_clock::now();
	for (int i = 0; i < testNumberCount; i++) {
		for (int j = 0; j < testNumberCount; j++) {
			testResult = testNumbers_int_limited[i] - testNumbers_int_limited[j];
		}
	}
	end_time = std::chrono::steady_clock::now();
	duration = end_time - start_time;
	std::cout << duration.count() << " seconds" << std::endl;
	results.push_back(duration.count());

	std::cout << "Measuring Multiplication: ";
	start_time = std::chrono::steady_clock::now();
	for (int i = 0; i < testNumberCount; i++) {
		for (int j = 0; j < testNumberCount; j++) {
			testResult = testNumbers_int_limited[i] * testNumbers_int_limited[j];
		}
	}
	end_time = std::chrono::steady_clock::now();
	duration = end_time - start_time;
	std::cout << duration.count() << " seconds" << std::endl;
	results.push_back(duration.count());
	
	std::cout << "Measuring Division: ";
	start_time = std::chrono::steady_clock::now();
	for (int i = 0; i < testNumberCount; i++) {
		for (int j = 0; j < testNumberCount; j++) {
			if (testNumbers_int_limited[j] == 0) continue;
			testResult = testNumbers_int_limited[i] / testNumbers_int_limited[j];
		}
	}
	end_time = std::chrono::steady_clock::now();
	duration = end_time - start_time;
	std::cout << duration.count() << " seconds" << std::endl;
	results.push_back(duration.count());

	std::cout << "Measuring Modulo: ";
	start_time = std::chrono::steady_clock::now();
	for (int i = 0; i < testNumberCount; i++) {
		for (int j = 0; j < testNumberCount; j++) {
			if (testNumbers_int_limited[j] == 0) continue;
			testResult = testNumbers_int_limited[i] % testNumbers_int_limited[j];
		}
	}
	end_time = std::chrono::steady_clock::now();
	duration = end_time - start_time;
	std::cout << duration.count() << " seconds" << std::endl;
	results.push_back(duration.count());

	// here to assure that compiler optimization won't skip loops
	if (testResult == -2) {
		std::cout << "An error has occurred" << std::endl;
	}

	return results;
}

template <int bitSize>
std::vector<double> averageBenchmark_int_limited(int testNumberCount = 5000, int iterations = 10) {
	std::cout << "====================================================================================================" << std::endl;
	std::cout << "Conducting " << iterations << " benchmarks of " << int_limited<bitSize>::className() << " on " << uint64_t(testNumberCount)*testNumberCount << " cases" << std::endl;
	
	uint64_t randState = 1;
	// addition, subtraction, multiplication, division, modulo
	std::vector<double> operationTimes = {0, 0, 0, 0, 0};
	std::vector<std::string> operationNames = {"Addition", "Subtraction", "Multiplication", "Division", "Modulo"};

	for (int i = 0; i < iterations; i++) {
		std::cout << std::endl << "Iteration " << i+1 << ":" << std::endl;
		auto results = speedBenchmark_int_limited<bitSize>(testNumberCount, randState);
		for (int j = 0; j < results.size(); j++) {
			operationTimes[j] += results[j];
		}
		randState += uint64_t(testNumberCount)*testNumberCount + 1;
	}
	std::cout << std::endl;
	for (int i = 0; i < operationTimes.size(); i++) {
		operationTimes[i] /= iterations;
		std::cout << "Average " << operationNames[i] << " time: " << operationTimes[i] << std::endl;
	}
	return operationTimes;
}

std::vector<double> speedBenchmarkBoost(int bits = 256, int testNumberCount = 5000, uint64_t randState = 1) {
	std::cout << "====================================================================================================" << std::endl;
	std::cout << "Benchmarking boost cpp_int with " << bits << " bit values on " << uint64_t(testNumberCount)*testNumberCount << " cases" << std::endl;

	std::vector<boostInt> testNumbersBoost = generateBoostIntegers(testNumberCount, bits, randState);
	std::vector<double> results;
	boostInt testResult = 0; // checked at the end to stop optimization from skipping loops
	
	std::cout << "Measuring Addition: ";
	auto start_time = std::chrono::steady_clock::now();
	for (int i = 0; i < testNumberCount; i++) {
		for (int j = 0; j < testNumberCount; j++) {
			testResult = testNumbersBoost[i] + testNumbersBoost[j];
		}
	}
	auto end_time = std::chrono::steady_clock::now();
	std::chrono::duration<double> duration(end_time - start_time);
	std::cout << duration.count() << " seconds" << std::endl;
	results.push_back(duration.count());

	std::cout << "Measuring Subtraction: ";
	start_time = std::chrono::steady_clock::now();
	for (int i = 0; i < testNumberCount; i++) {
		for (int j = 0; j < testNumberCount; j++) {
			testResult = testNumbersBoost[i] - testNumbersBoost[j];
		}
	}
	end_time = std::chrono::steady_clock::now();
	duration = end_time - start_time;
	std::cout << duration.count() << " seconds" << std::endl;
	results.push_back(duration.count());

	std::cout << "Measuring Multiplication: ";
	start_time = std::chrono::steady_clock::now();
	for (int i = 0; i < testNumberCount; i++) {
		for (int j = 0; j < testNumberCount; j++) {
			testResult = testNumbersBoost[i] * testNumbersBoost[j];
		}
	}
	end_time = std::chrono::steady_clock::now();
	duration = end_time - start_time;
	std::cout << duration.count() << " seconds" << std::endl;
	results.push_back(duration.count());
	
	std::cout << "Measuring Division: ";
	start_time = std::chrono::steady_clock::now();
	for (int i = 0; i < testNumberCount; i++) {
		for (int j = 0; j < testNumberCount; j++) {
			if (testNumbersBoost[j] == 0) continue;
			testResult = testNumbersBoost[i] / testNumbersBoost[j];
		}
	}
	end_time = std::chrono::steady_clock::now();
	duration = end_time - start_time;
	std::cout << duration.count() << " seconds" << std::endl;
	results.push_back(duration.count());

	std::cout << "Measuring Modulo: ";
	start_time = std::chrono::steady_clock::now();
	for (int i = 0; i < testNumberCount; i++) {
		for (int j = 0; j < testNumberCount; j++) {
			if (testNumbersBoost[j] == 0) continue;
			testResult = testNumbersBoost[i] % testNumbersBoost[j];
		}
	}
	end_time = std::chrono::steady_clock::now();
	duration = end_time - start_time;
	std::cout << duration.count() << " seconds" << std::endl;
	results.push_back(duration.count());

	if (testResult == -2) {
		std::cout << "An error has occurred" << std::endl;
	}

	return results;
}

std::vector<double> averageBenchmarkBoost(int bits = 256, int testNumberCount = 5000, int iterations = 10) {
	std::cout << "====================================================================================================" << std::endl;
	std::cout << "Conducting " << iterations << " benchmarks of Boost cpp_int with" << bits << " bit values on " << uint64_t(testNumberCount)*testNumberCount << " cases" << std::endl;
	
	uint64_t randState = 1;
	// addition, subtraction, multiplication, division, modulo
	std::vector<double> operationTimes = {0, 0, 0, 0, 0};
	std::vector<std::string> operationNames = {"Addition", "Subtraction", "Multiplication", "Division", "Modulo"};

	for (int i = 0; i < iterations; i++) {
		std::cout << std::endl << "Iteration " << i+1 << ":" << std::endl;
		auto results = speedBenchmarkBoost(bits, testNumberCount, randState);
		for (int j = 0; j < results.size(); j++) {
			operationTimes[j] += results[j];
		}
		randState += uint64_t(testNumberCount)*testNumberCount + 1;
	}
	std::cout << std::endl;
	for (int i = 0; i < operationTimes.size(); i++) {
		operationTimes[i] /= iterations;
		std::cout << "Average " << operationNames[i] << " time: " << operationTimes[i] << std::endl;
	}
	return operationTimes;
}

void formatBenchmarkTimesTable(std::vector<std::vector<double>>& averageBenchmarkResults, std::vector<std::string>& column_names, std::vector<std::string>& row_names) {
	assert(column_names.size() > 0);

	std::printf("| %-18s|", column_names[0].c_str());
	for (int i = 1; i < column_names.size(); i++) {
		printf(" %-18s|", column_names[i].c_str());
	}

	printf("\n|");
	for (int buffer = 0; buffer < column_names.size(); buffer++) {
		printf(" %-18s|", "---");
	}

	printf("\n");
	for (int i = 0; i < averageBenchmarkResults.size(); i++) {
		printf("| %-18s|", row_names[i].c_str());

		assert(averageBenchmarkResults[i].size() == column_names.size()-1);
		for (int j = 0; j < averageBenchmarkResults[i].size(); j++) {
			printf(" %-18.4f|", averageBenchmarkResults[i][j]);
		}
		printf("\n");
	}
}

void benchmark_int_limited(int testCaseAmount = 5000, uint64_t randState = 1) {
	std::vector<std::vector<double>> benchmarkResults;
	std::vector<std::string> column_names = {"Type", "Addition", "Subtraction", "Multiplication", "Division", "Modulo"};
	std::vector<std::string> row_names;

	// Tests multiples of 32 and different offsets
	// benchmarkResults.push_back(averageBenchmark_int_limited<16>(testCaseAmount));
	// row_names.push_back("int_limited<16>");
	// benchmarkResults.push_back(averageBenchmark_int_limited<17>(testCaseAmount));
	// row_names.push_back("int_limited<17>");
	// benchmarkResults.push_back(averageBenchmark_int_limited<32>(testCaseAmount));
	// row_names.push_back("int_limited<32>");
	// benchmarkResults.push_back(averageBenchmark_int_limited<37>(testCaseAmount));
	// row_names.push_back("int_limited<37>");
	// benchmarkResults.push_back(averageBenchmark_int_limited<32*2>(testCaseAmount));
	// row_names.push_back("int_limited<64>");
	// benchmarkResults.push_back(averageBenchmark_int_limited<32*2 + 1>(testCaseAmount));
	// row_names.push_back("int_limited<65>");
	// benchmarkResults.push_back(averageBenchmark_int_limited<32*4>(testCaseAmount));
	// row_names.push_back("int_limited<128>");
	// benchmarkResults.push_back(averageBenchmark_int_limited<32*4 + 2>(testCaseAmount));
	// row_names.push_back("int_limited<130>");


	// benchmarkResults.push_back(averageBenchmark_int_limited<32*6>(testCaseAmount));
	// row_names.push_back("int_limited<196>");
	// benchmarkResults.push_back(averageBenchmark_int_limited<32*6 + 3>(testCaseAmount));
	// row_names.push_back("int_limited<199>");
	// benchmarkResults.push_back(averageBenchmark_int_limited<32*8>(testCaseAmount));
	// row_names.push_back("int_limited<256>");
	// benchmarkResults.push_back(averageBenchmark_int_limited<32*8 + 6>(testCaseAmount));
	// row_names.push_back("int_limited<262>");
	// benchmarkResults.push_back(averageBenchmark_int_limited<32*20>(testCaseAmount));
	// row_names.push_back("int_limited<640>");
	// benchmarkResults.push_back(averageBenchmark_int_limited<32*20 + 17>(testCaseAmount));
	// row_names.push_back("int_limited<657>");
	// benchmarkResults.push_back(averageBenchmark_int_limited<32*32>(testCaseAmount));
	// row_names.push_back("int_limited<1024>");
	// benchmarkResults.push_back(averageBenchmark_int_limited<32*32 + 31>(testCaseAmount));
	// row_names.push_back("int_limited<1055>");

	// testCaseAmount = 2000;
	// benchmarkResults.push_back(averageBenchmark_int_limited<32*42>(testCaseAmount));
	// row_names.push_back("int_limited<1344>");
	// benchmarkResults.push_back(averageBenchmark_int_limited<32*42 + 16>(testCaseAmount));
	// row_names.push_back("int_limited<1360>");
	// benchmarkResults.push_back(averageBenchmark_int_limited<32*64>(testCaseAmount));
	// row_names.push_back("int_limited<2048>");
	// benchmarkResults.push_back(averageBenchmark_int_limited<32*64 + 59>(testCaseAmount));
	// row_names.push_back("int_limited<2107>");
	// benchmarkResults.push_back(averageBenchmark_int_limited<32*128>(testCaseAmount));
	// row_names.push_back("int_limited<4096>");
	// benchmarkResults.push_back(averageBenchmark_int_limited<32*128 + 63>(testCaseAmount));
	// row_names.push_back("int_limited<4159>");

	testCaseAmount = 200;
	benchmarkResults.push_back(averageBenchmark_int_limited<32*512>(testCaseAmount));
	row_names.push_back("int_limited<16384>");
	benchmarkResults.push_back(averageBenchmark_int_limited<32*1024>(testCaseAmount));
	row_names.push_back("int_limited<32768>");
	benchmarkResults.push_back(averageBenchmark_int_limited<32*2048>(testCaseAmount));
	row_names.push_back("int_limited<65536>");

	formatBenchmarkTimesTable(benchmarkResults, column_names, row_names);
}

void benchmarkBoost(int testCaseAmount = 5000, uint64_t randState = 1) {
	std::vector<std::vector<double>> benchmarkResults;
	std::vector<std::string> column_names = {"Type", "Addition", "Subtraction", "Multiplication", "Division", "Modulo"};
	std::vector<std::string> row_names;

	// Tests multiples of 32 and different offsets
	// benchmarkResults.push_back(averageBenchmarkBoost(16, testCaseAmount));
	// row_names.push_back("cpp_int (16 bits)");
	// benchmarkResults.push_back(averageBenchmarkBoost(17, testCaseAmount));
	// row_names.push_back("cpp_int (17 bits)");
	// benchmarkResults.push_back(averageBenchmarkBoost(32, testCaseAmount));
	// row_names.push_back("cpp_int (32 bits)");
	// benchmarkResults.push_back(averageBenchmarkBoost(37, testCaseAmount));
	// row_names.push_back("cpp_int (37 bits)");
	// benchmarkResults.push_back(averageBenchmarkBoost(64, testCaseAmount));
	// row_names.push_back("cpp_int (64 bits)");
	// benchmarkResults.push_back(averageBenchmarkBoost(65, testCaseAmount));
	// row_names.push_back("cpp_int (65 bits)");
	// benchmarkResults.push_back(averageBenchmarkBoost(128, testCaseAmount));
	// row_names.push_back("cpp_int (128 bits)");
	// benchmarkResults.push_back(averageBenchmarkBoost(130, testCaseAmount));
	// row_names.push_back("cpp_int (130 bits)");


	// benchmarkResults.push_back(averageBenchmarkBoost(196, testCaseAmount));
	// row_names.push_back("cpp_int (196 bits)");
	// benchmarkResults.push_back(averageBenchmarkBoost(199, testCaseAmount));
	// row_names.push_back("cpp_int (199 bits)");
	// benchmarkResults.push_back(averageBenchmarkBoost(256, testCaseAmount));
	// row_names.push_back("cpp_int (256 bits)");
	// benchmarkResults.push_back(averageBenchmarkBoost(262, testCaseAmount));
	// row_names.push_back("cpp_int (262 bits)");
	// benchmarkResults.push_back(averageBenchmarkBoost(640, testCaseAmount));
	// row_names.push_back("cpp_int (640 bits)");
	// benchmarkResults.push_back(averageBenchmarkBoost(657, testCaseAmount));
	// row_names.push_back("cpp_int (657 bits)");
	// benchmarkResults.push_back(averageBenchmarkBoost(1024, testCaseAmount));
	// row_names.push_back("cpp_int (1024 bits)");
	// benchmarkResults.push_back(averageBenchmarkBoost(1055, testCaseAmount));
	// row_names.push_back("cpp_int (1055 bits)");

	// testCaseAmount = 2000;
	// benchmarkResults.push_back(averageBenchmarkBoost(1344, testCaseAmount));
	// row_names.push_back("cpp_int (1344 bits)");
	// benchmarkResults.push_back(averageBenchmarkBoost(1360, testCaseAmount));
	// row_names.push_back("cpp_int (1360 bits)");
	// benchmarkResults.push_back(averageBenchmarkBoost(2048, testCaseAmount));
	// row_names.push_back("cpp_int (2048 bits)");
	// benchmarkResults.push_back(averageBenchmarkBoost(2107, testCaseAmount));
	// row_names.push_back("cpp_int (2107 bits)");
	// benchmarkResults.push_back(averageBenchmarkBoost(4096, testCaseAmount));
	// row_names.push_back("cpp_int (4096 bits)");
	// benchmarkResults.push_back(averageBenchmarkBoost(4159, testCaseAmount));
	// row_names.push_back("cpp_int (4159 bits)");

	testCaseAmount = 200;
	benchmarkResults.push_back(averageBenchmarkBoost(16384, testCaseAmount));
	row_names.push_back("cpp_int (16384 bits)");
	benchmarkResults.push_back(averageBenchmarkBoost(32768, testCaseAmount));
	row_names.push_back("cpp_int (32768 bits)");
	benchmarkResults.push_back(averageBenchmarkBoost(65536, testCaseAmount));
	row_names.push_back("cpp_int (65536 bits)");

	formatBenchmarkTimesTable(benchmarkResults, column_names, row_names);
}

int main() {
	benchmarkBoost();
	return 0;
}