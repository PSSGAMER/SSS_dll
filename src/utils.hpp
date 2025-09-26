#pragma once

#include <string>
#include <vector>

namespace Utils
{
	std::vector<std::string> strsplit(const std::string& str, const std::string& delimeter);
	std::string getFileSHA256(const char* filePath);
}