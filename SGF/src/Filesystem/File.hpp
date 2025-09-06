#pragma once

#include "SGF_Core.hpp"


namespace SGF {
	std::vector<char> LoadBinaryFile(const char* filename);
	size_t LoadBinaryFileToBuffer(const char* filename, size_t bufSize, char* pBuf);

	bool SaveBinaryFile(const char* filename, size_t dataSize, const char* pData);
	bool SaveBinaryFile(const char* filename, const std::vector<char>& data);

	std::vector<uint8_t> LoadTextureFile(const char* filename, uint32_t* pWidth, uint32_t* pHeight);

	std::vector<uint8_t> LoadTextureFromMemory(const uint8_t* data, uint32_t* pWidth, uint32_t* pHeight);

	std::string GetDirectoryFromFilePath(const char* filePath);
}