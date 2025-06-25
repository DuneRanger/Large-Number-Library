#pragma once
#include <vector>
#include "../int_limited.hpp"

class QuadraticSieve {
	public:
		static std::vector<int512> factorise(int512 value);
		static std::vector<int512> factorise(std::string value);
}
