#pragma once
#include <cstdint>
#include <iostream>
#include <string>
#include <vector>
#include "./int128.hpp"


namespace customBigInt {
	// Two's complement
	// Constructs a vector of 64 bit unsigned integers, so that the specified bit size fits
	// If the bitSize isn't a multiple of 64, operations will still be processed for all 64 bits of the most significant word
	// However overflow will still occur if the value were to surpass the bitSize
	// Comparisons will also ignore any extra bits above the bitSize
	// No further optimizations are made on the most significant word (even if the instance only has 1 word)
	template <int bitSize>
	class int_limited {
	};
}