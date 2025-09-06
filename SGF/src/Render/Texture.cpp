#include "Texture.hpp"

#include <stb_image.h>

namespace SGF {
    Texture::Texture(const char* filename) {
        int channels;
        pixels = stbi_load(filename, (int*)&area.width, (int*)&area.height, (int*)&channels, STBI_rgb_alpha);
        if (!pixels) fatal("Failed to load texture!");
    }
    Texture::Texture(const uint8_t* buffer, uint32_t bufferSize) {
        int channels;
        pixels = stbi_load_from_memory(buffer, bufferSize, (int*)&area.width, (int*)&area.height, (int*)&channels, STBI_rgb_alpha);
        if (!pixels) fatal("Failed to load texture!");
    }
    Texture::Texture(uint32_t width, uint32_t height, const uint8_t* data) : area{width, height} {
        pixels = new uint8_t[width * height * 4];
        memcpy(pixels, data, width * height * 4);
    }
    Texture::Texture(const Texture& other) {
        size_t memsize = other.GetMemorySize();
        pixels = (uint8_t*)malloc(memsize);
        if (!pixels) fatal("Failed to allocate memory!");
        memcpy(pixels, other.pixels, memsize);
        area = other.area;
        //channelCount = other.channelCount;
    }
    Texture::Texture(Texture&& other) {
        pixels = other.pixels;
        area = other.area;
        //channelCount = other.channelCount;
        other.pixels = nullptr;
    }
    
} // namespace SGF
