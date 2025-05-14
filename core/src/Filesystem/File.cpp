#include "Filesystem/File.hpp"

#include <fstream>

namespace SGF {
	std::vector<char> loadBinaryFile(const char* filename) {
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
	size_t loadBinaryFileToBuffer(const char* filename, size_t bufSize, char* pBuf) {
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
	bool saveBinaryFile(const char* filename, size_t dataSize, const char* pData) {
		std::ofstream file(filename, std::ios::ate | std::ios::binary);
		if (!file.is_open()) {
			SGF::error("Failed to open file: ", filename);
			return false;
		}
		file.write(pData, dataSize);
		file.close();
		return true;
	}

	bool saveBinaryFile(const char* filename, const std::vector<char>& data) {
		return saveBinaryFile(filename, data.size(), data.data());
	}
	std::vector<uint8_t> loadTextureFile(const char* filename, uint32_t* pWidth, uint32_t* pHeight) {
		return std::vector<uint8_t>();
	}
}