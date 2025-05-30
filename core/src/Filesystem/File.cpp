#include "Filesystem/File.hpp"

#include <fstream>

namespace SGF {
	std::vector<char> LoadBinaryFile(const char* filename) {
		std::ifstream file(filename, std::ios::ate | std::ios::binary);
		std::vector<char> buffer;
		if (!file.is_open()) {
			SGF::error("Failed to open file: ", filename);
			return buffer;
		}

		size_t fileSize = (size_t)file.tellg();
		buffer.resize(fileSize);

		file.seekg(0);
		file.read(buffer.data(), fileSize);
		file.close();
		return buffer;
	}
	size_t LoadBinaryFileToBuffer(const char* filename, size_t bufSize, char* pBuf) {
		std::ifstream file(filename, std::ios::ate | std::ios::binary);
		if (!file.is_open()) {
			SGF::error("Failed to open file: ", filename);
			return 0;
		}
		size_t fileSize = (size_t)file.tellg();
		if (fileSize <= bufSize) {
			file.seekg(0);
			file.read(pBuf, fileSize);
		}
		else {
			SGF::error("Buffer is smaller than the file-size!");
			fileSize = 0;
		}
		file.close();
		return fileSize;
	}
	bool SaveBinaryFile(const char* filename, size_t dataSize, const char* pData) {
		std::ofstream file(filename, std::ios::ate | std::ios::binary);
		if (!file.is_open()) {
			SGF::error("Failed to open file: ", filename);
			return false;
		}
		file.write(pData, dataSize);
		file.close();
		return true;
	}

	bool SaveBinaryFile(const char* filename, const std::vector<char>& data) {
		return SaveBinaryFile(filename, data.size(), data.data());
	}
	std::vector<uint8_t> LoadTextureFile(const char* filename, uint32_t* pWidth, uint32_t* pHeight) {
		return std::vector<uint8_t>();
	}
}