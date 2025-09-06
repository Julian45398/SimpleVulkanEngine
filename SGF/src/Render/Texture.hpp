#pragma once

#include "SGF_Core.hpp"


namespace SGF {
    struct Area2D {
		uint32_t width;
		uint32_t height;
	};
	struct Offset2D {
		int32_t x;
		int32_t y;
	};
	struct Rectangle {
		Area2D area;
		Offset2D offset;
	};
    class Texture {
		uint8_t* pixels;
		Area2D area;
	public:
		Texture(const char* filename);
		Texture(const uint8_t* buffer, uint32_t bufferSize);
        Texture(uint32_t width, uint32_t height, const uint8_t* pixels);
		Texture(const Texture& other);
		Texture(Texture&& other);
		inline ~Texture() { if (pixels) free(pixels); }
		inline Area2D GetSize() const { return area; }
		inline uint32_t GetWidth() const { return area.width; }
		inline uint32_t GetHeight() const { return area.height; }
		inline size_t GetMemorySize() const { return area.width * area.height * 4; }
		inline const uint8_t* GetData() const { return pixels; }
	};
}