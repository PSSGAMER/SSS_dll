#include "utils.hpp"

#include <fstream>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>

#include <windows.h>
#include <bcrypt.h>
#pragma comment(lib, "bcrypt.lib")

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

struct BCryptAlgHandle
{
    BCRYPT_ALG_HANDLE handle = NULL;
    ~BCryptAlgHandle() { if (handle) BCryptCloseAlgorithmProvider(handle, 0); }
};

struct BCryptHashHandle
{
    BCRYPT_HASH_HANDLE handle = NULL;
    ~BCryptHashHandle() { if (handle) BCryptDestroyHash(handle); }
};

std::string Utils::getFileSHA256(const char *filePath)
{
    std::ifstream fs(filePath, std::ios::binary);
    if (!fs.is_open())
    {
        throw std::runtime_error("Unable to open file!");
    }

    BCryptAlgHandle hAlg;
    NTSTATUS status = BCryptOpenAlgorithmProvider(&hAlg.handle, BCRYPT_SHA256_ALGORITHM, NULL, 0);
    if (!BCRYPT_SUCCESS(status))
    {
        throw std::runtime_error("BCryptOpenAlgorithmProvider failed.");
    }

    BCryptHashHandle hHash;
    status = BCryptCreateHash(hAlg.handle, &hHash.handle, NULL, 0, NULL, 0, 0);
    if (!BCRYPT_SUCCESS(status))
    {
        throw std::runtime_error("BCryptCreateHash failed.");
    }

    // Read and hash the file in chunks for better memory efficiency.
    const size_t bufferSize = 4096;
    std::vector<char> buffer(bufferSize);
    while (fs.good())
    {
        fs.read(buffer.data(), bufferSize);
        std::streamsize bytesRead = fs.gcount();
        if (bytesRead > 0)
        {
            status = BCryptHashData(hHash.handle, (PBYTE)buffer.data(), (ULONG)bytesRead, 0);
            if (!BCRYPT_SUCCESS(status))
            {
                fs.close();
                throw std::runtime_error("BCryptHashData failed.");
            }
        }
    }
    fs.close();

    // Get the required size for the hash digest.
    DWORD cbHash = 0;
    DWORD cbData = sizeof(DWORD);
    status = BCryptGetProperty(hAlg.handle, BCRYPT_HASH_LENGTH, (PBYTE)&cbHash, cbData, &cbData, 0);
    if (!BCRYPT_SUCCESS(status))
    {
        throw std::runtime_error("BCryptGetProperty failed to get hash length.");
    }

    // Finalize the hash and get the result.
    std::vector<BYTE> hashBytes(cbHash);
    status = BCryptFinishHash(hHash.handle, hashBytes.data(), cbHash, 0);
    if (!BCRYPT_SUCCESS(status))
    {
        throw std::runtime_error("BCryptFinishHash failed.");
    }

    // Convert the raw hash bytes to a lowercase hex string.
    std::stringstream sha256;
    sha256 << std::hex << std::setfill('0');
    for (BYTE byte : hashBytes)
    {
        sha256 << std::setw(2) << static_cast<int>(byte);
    }

    return sha256.str();
}