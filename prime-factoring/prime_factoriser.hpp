#pragma once
#include <vector>
#include "../int_limited.hpp"

class Factoriser {
	public:
		static std::vector<int512> factorise(int512 value);
		static std::vector<int512> factorise(std::string value);
}