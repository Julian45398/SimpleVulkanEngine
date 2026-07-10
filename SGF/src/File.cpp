#include <SGF/Core/Platform/File.hpp>

#include <SGF/Core/Debugging/Logger.hpp>

#include <fstream>
#include <filesystem>
#include <stb_image.h>
#include <SGF/Core/Macros.hpp>

namespace SGF {
	namespace File {
		std::vector<char> LoadBinary(const char* filename) {
			std::ifstream file(filename, std::ios::ate | std::ios::binary);
			std::vector<char> buffer;
			if (!file.is_open()) {
				SGF::Log::Warn("Failed to open file: {}", filename);
				return buffer;
			}

			size_t fileSize = (size_t)file.tellg();
			buffer.resize(fileSize);

			file.seekg(0);
			file.read(buffer.data(), fileSize);
			file.close();
			return buffer;
		}
		size_t LoadBinaryToBuffer(const char* filename, size_t bufSize, char* pBuf) {
			std::ifstream file(filename, std::ios::ate | std::ios::binary);
			if (!file.is_open()) {
				SGF::Log::Error("Failed to open file: {}", filename);
				return 0;
			}
			size_t fileSize = (size_t)file.tellg();
			if (fileSize <= bufSize) {
				file.seekg(0);
				file.read(pBuf, fileSize);
			}
			else {
				SGF::Log::Error("Buffer is smaller than the file-size!");
				fileSize = 0;
			}
			file.close();
			return fileSize;
		}
		bool SaveBinary(const char* filename, size_t dataSize, const char* pData) {
			std::ofstream file(filename, std::ios::ate | std::ios::binary);
			if (!file.is_open()) {
				SGF::Log::Error("Failed to open file: {}", filename);
				return false;
			}
			file.write(pData, dataSize);
			file.close();
			return true;
		}

		std::string GetDirectoryFromPath(const char* filePath) {
			std::filesystem::path p(filePath);
			return p.parent_path().string();
		}

		bool SaveBinary(const char* filename, const std::vector<char>& data) {
			return SaveBinary(filename, data.size(), data.data());
		}
		std::vector<uint8_t> LoadTexture(const char* filename, uint32_t* pWidth, uint32_t* pHeight) {
			int channels;
			auto pixels = stbi_load(filename, (int*)pWidth, (int*)pHeight, &channels, STBI_rgb_alpha);
			SGF_ASSERT(channels == STBI_rgb_alpha);
			std::vector<uint8_t> data(pixels, pixels + (*pWidth) * (*pHeight) * channels);
			stbi_image_free(pixels);
			return data;
		}
		std::vector<uint8_t> LoadTextureFromMemory(const char* filename, uint32_t* pWidth, uint32_t* pHeight) {
			int channels;
			auto pixels = stbi_load(filename, (int*)pWidth, (int*)pHeight, &channels, STBI_rgb_alpha);
			SGF_ASSERT(channels == STBI_rgb_alpha);
			std::vector<uint8_t> data(pixels, pixels + (*pWidth) * (*pHeight) * channels);
			stbi_image_free(pixels);
			return data;
		}
	}
	
}