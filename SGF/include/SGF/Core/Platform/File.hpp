#pragma once

#include <stdint.h>
#include <vector>
#include <string>

namespace SGF {
	namespace File {
		std::vector<char> LoadBinary(const char* filename);
		inline std::vector<char> LoadBinary(const std::string& filename) { return LoadBinary(filename.c_str()); }
		size_t LoadBinaryToBuffer(const char* filename, size_t bufSize, char* pBuf);
		inline size_t LoadBinaryToBuffer(const std::string& filename, size_t bufSize, char* pBuf) { return LoadBinaryToBuffer(filename.c_str(), bufSize, pBuf); }

		bool SaveBinary(const char* filename, size_t dataSize, const char* pData);
		inline bool SaveBinary(const std::string& filename, size_t dataSize, const char* pData) { return SaveBinary(filename.c_str(), dataSize, pData); }
		bool SaveBinary(const char* filename, const std::vector<char>& data);
		inline bool SaveBinary(const std::string& filename, const std::vector<char>& data) { return SaveBinary(filename.c_str(), data); }

		std::vector<uint8_t> LoadTexture(const char* filename, uint32_t* pWidth, uint32_t* pHeight);
		inline std::vector<uint8_t> LoadTexture(const std::string& filename, uint32_t* pWidth, uint32_t* pHeight) { return LoadTexture(filename.c_str(), pWidth, pHeight); }

		std::vector<uint8_t> LoadTextureFromMemory(const uint8_t* pData, size_t dataSize, uint32_t* pWidth, uint32_t* pHeight);

		bool SaveTexture(const char* filename, uint32_t width, uint32_t height, const uint8_t* data);
		inline bool SaveTexture(const std::string& filename, uint32_t width, uint32_t height, const uint8_t* data) { return SaveTexture(filename.c_str(), width, height, data); }

		std::string GetDirectoryFromPath(const char* filePath);
		inline std::string GetDirectoryFromPath(const std::string& filePath) { return GetDirectoryFromPath(filePath.c_str()); }
	}
}