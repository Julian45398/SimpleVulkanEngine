#pragma once

#include "SGF_Core.hpp"

namespace SGF {
	std::vector<char> loadBinaryFile(const char* filename);
	size_t loadBinaryFileToBuffer(const char* filename, size_t bufSize, char* pBuf);

	bool saveBinaryFile(const char* filename, size_t dataSize, const char* pData);
	bool saveBinaryFile(const char* filename, const std::vector<char>& data);

	std::vector<uint8_t> loadTextureFile(const char* filename, uint32_t* pWidth, uint32_t* pHeight);
}