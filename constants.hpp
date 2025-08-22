#pragma once
#include <cstdint>


namespace largeNumberLibrary {
	// equivalent to INT32_MIN
	constexpr uint32_t BIT32_ON = 0x80000000;

	// equivalent to INT64_MAX
	constexpr uint64_t UINT63_MAX = 0x7FFFFFFFFFFFFFFF;
	// equivalent to INT64_MIN
	constexpr uint64_t BIT64_ON = 0x8000000000000000;
}