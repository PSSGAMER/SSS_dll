#include "utils.hpp"

#include <fstream>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>
#include <openssl/sha.h>

std::vector<std::string> Utils::strsplit(const std::string& str, const std::string& delimeter)
{
    std::vector<std::string> splits;
    size_t start_pos = 0;
    size_t end_pos;

    while ((end_pos = str.find(delimeter, start_pos)) != std::string::npos)
    {
        splits.push_back(str.substr(start_pos, end_pos - start_pos));
        start_pos = end_pos + delimeter.length();
    }

    splits.push_back(str.substr(start_pos));

    return splits;
}

std::string Utils::getFileSHA256(const char *filePath)
{
	std::ifstream fs(filePath, std::ios::binary);
	if (!fs.is_open())
	{
		//TODO: Read more about error types in C++ :)
		throw std::runtime_error("Unable to read file!");
	}

	std::vector<unsigned char> bytes(std::istreambuf_iterator<char>(fs), {});
	unsigned char sha256Bytes[SHA256_DIGEST_LENGTH];
	SHA256(bytes.data(), bytes.size(), sha256Bytes);

	std::stringstream sha256;
	for(int i = 0; i < SHA256_DIGEST_LENGTH; i++)
	{
		sha256 << std::hex << std::setw(2) << std::setfill('0') << (int)sha256Bytes[i];
	}

	fs.close();
	return sha256.str();
}