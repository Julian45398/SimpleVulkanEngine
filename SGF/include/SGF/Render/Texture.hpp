#pragma once

#include <stdint.h>
#include <glm/glm.hpp>

namespace SGF {
	class Texture {
	private:
		char* m_Pixels;
		glm::uvec2 m_Size;
		uint32_t m_PixelSize;
	};
}