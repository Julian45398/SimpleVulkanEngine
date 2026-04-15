#pragma once

#include "SGF_Core.hpp"

namespace SGF {
	namespace Color {
		// Forward declarations
		class RGBA8;
		class RGB8;
		class RGBA32;
		class RGB32;

		// ==================== RGBA8 ====================
		class RGBA8 {
		public:
			uint8_t r, g, b, a;

			inline void SetR(float r) { this->r = static_cast<uint8_t>(glm::clamp(r, 0.0f, 1.0f) * 255.0f); }
			inline void SetG(float g) { this->g = static_cast<uint8_t>(glm::clamp(g, 0.0f, 1.0f) * 255.0f); }
			inline void SetB(float b) { this->b = static_cast<uint8_t>(glm::clamp(b, 0.0f, 1.0f) * 255.0f); }
			inline void SetA(float a) { this->a = static_cast<uint8_t>(glm::clamp(a, 0.0f, 1.0f) * 255.0f); }

			inline void SetR(uint8_t red) { r = red; }
			inline void SetG(uint8_t green) { g = green; }
			inline void SetB(uint8_t blue) { b = blue; }
			inline void SetA(uint8_t alpha) { a = alpha; }

			inline glm::vec4 ToVec4() const {
				return glm::vec4(r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f);
			}

			inline RGBA8(float r, float g, float b, float a = 1.0f) {
				SetR(r);
				SetG(g);
				SetB(b);
				SetA(a);
			}
			inline RGBA8(const glm::vec4& color) {
				SetR(color.r);
				SetG(color.g);
				SetB(color.b);
				SetA(color.a);
			}
			inline RGBA8(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255) : r(r), g(g), b(b), a(a) {}
			inline RGBA8() : r(0), g(0), b(0), a(255) {}

			// Conversion operators
			inline operator glm::vec4() const { return ToVec4(); }

			inline static const RGBA8 White() { return RGBA8((uint8_t)255, (uint8_t)255, (uint8_t)255, (uint8_t)255); }
			inline static const RGBA8 Black() { return RGBA8((uint8_t)0, (uint8_t)0, (uint8_t)0, (uint8_t)255); }
			inline static const RGBA8 Red() { return RGBA8((uint8_t)255, (uint8_t)0, (uint8_t)0, (uint8_t)255); }
			inline static const RGBA8 Green() { return RGBA8((uint8_t)0, (uint8_t)255, (uint8_t)0, (uint8_t)255); }
			inline static const RGBA8 Blue() { return RGBA8((uint8_t)0, (uint8_t)0, (uint8_t)255, (uint8_t)255); }
		};

		// ==================== RGB8 ====================
		class RGB8 {
		public:
			uint8_t r, g, b;

			inline void SetR(float r) { this->r = static_cast<uint8_t>(glm::clamp(r, 0.0f, 1.0f) * 255.0f); }
			inline void SetG(float g) { this->g = static_cast<uint8_t>(glm::clamp(g, 0.0f, 1.0f) * 255.0f); }
			inline void SetB(float b) { this->b = static_cast<uint8_t>(glm::clamp(b, 0.0f, 1.0f) * 255.0f); }

			inline void SetR(uint8_t red) { r = red; }
			inline void SetG(uint8_t green) { g = green; }
			inline void SetB(uint8_t blue) { b = blue; }

			inline glm::vec3 ToVec3() const {
				return glm::vec3(r / 255.0f, g / 255.0f, b / 255.0f);
			}

			inline RGB8(float r, float g, float b) {
				SetR(r);
				SetG(g);
				SetB(b);
			}
			inline RGB8(const glm::vec3& color) {
				SetR(color.r);
				SetG(color.g);
				SetB(color.b);
			}
			inline RGB8(uint8_t r, uint8_t g, uint8_t b) : r(r), g(g), b(b) {}
			inline RGB8() : r(0), g(0), b(0) {}

			// Conversion operators
			inline operator glm::vec3() const { return ToVec3(); }
		};

		// ==================== RGBA32 ====================
		class RGBA32 {
		public:
			float r, g, b, a;

			inline void SetR(float r) { this->r = glm::clamp(r, 0.0f, 1.0f); }
			inline void SetG(float g) { this->g = glm::clamp(g, 0.0f, 1.0f); }
			inline void SetB(float b) { this->b = glm::clamp(b, 0.0f, 1.0f); }
			inline void SetA(float a) { this->a = glm::clamp(a, 0.0f, 1.0f); }

			inline void SetR(uint8_t r) { this->r = r / 255.0f; }
			inline void SetG(uint8_t g) { this->g = g / 255.0f; }
			inline void SetB(uint8_t b) { this->b = b / 255.0f; }
			inline void SetA(uint8_t a) { this->a = a / 255.0f; }

			inline glm::vec4 ToVec4() const {
				return glm::vec4(r, g, b, a);
			}

			inline RGBA32(float r, float g, float b, float a = 1.0f) {
				SetR(r);
				SetG(g);
				SetB(b);
				SetA(a);
			}
			inline RGBA32(const glm::vec4& color) {
				SetR(color.r);
				SetG(color.g);
				SetB(color.b);
				SetA(color.a);
			}
			inline RGBA32(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255) {
				SetR(r);
				SetG(g);
				SetB(b);
				SetA(a);
			}
			inline RGBA32() : r(0.0f), g(0.0f), b(0.0f), a(1.0f) {}

			// Conversion operators
			inline operator glm::vec4() const { return ToVec4(); }
		};

		// ==================== RGB32 ====================
		class RGB32 {
		public:
			float r, g, b;

			inline void SetR(float r) { this->r = glm::clamp(r, 0.0f, 1.0f); }
			inline void SetG(float g) { this->g = glm::clamp(g, 0.0f, 1.0f); }
			inline void SetB(float b) { this->b = glm::clamp(b, 0.0f, 1.0f); }

			inline void SetR(uint8_t r) { this->r = r / 255.0f; }
			inline void SetG(uint8_t g) { this->g = g / 255.0f; }
			inline void SetB(uint8_t b) { this->b = b / 255.0f; }

			inline glm::vec3 ToVec3() const {
				return glm::vec3(r, g, b);
			}

			inline RGB32(float r, float g, float b) {
				SetR(r);
				SetG(g);
				SetB(b);
			}
			inline RGB32(const glm::vec3& color) {
				SetR(color.r);
				SetG(color.g);
				SetB(color.b);
			}
			inline RGB32(uint8_t r, uint8_t g, uint8_t b) {
				SetR(r);
				SetG(g);
				SetB(b);
			}
			inline RGB32() : r(0.0f), g(0.0f), b(0.0f) {}

			// Conversion operators
			inline operator glm::vec3() const { return ToVec3(); }
		};

		// ==================== Conversion Functions ====================
		namespace Color {
			// RGBA8 conversions
			inline RGBA8 ConvertToRGBA8(const RGBA32& color) {
				return RGBA8(
					static_cast<uint8_t>(color.r * 255.0f),
					static_cast<uint8_t>(color.g * 255.0f),
					static_cast<uint8_t>(color.b * 255.0f),
					static_cast<uint8_t>(color.a * 255.0f)
				);
			}

			inline RGBA8 ConvertToRGBA8(const RGB8& color) {
				return RGBA8(color.r, color.g, color.b, 255);
			}

			inline RGBA8 ConvertToRGBA8(const RGB32& color) {
				return RGBA8(
					static_cast<uint8_t>(color.r * 255.0f),
					static_cast<uint8_t>(color.g * 255.0f),
					static_cast<uint8_t>(color.b * 255.0f),
					255
				);
			}

			// RGB8 conversions
			inline RGB8 ConvertToRGB8(const RGBA8& color) {
				return RGB8(color.r, color.g, color.b);
			}

			inline RGB8 ConvertToRGB8(const RGBA32& color) {
				return RGB8(
					static_cast<uint8_t>(color.r * 255.0f),
					static_cast<uint8_t>(color.g * 255.0f),
					static_cast<uint8_t>(color.b * 255.0f)
				);
			}

			inline RGB8 ConvertToRGB8(const RGB32& color) {
				return RGB8(
					static_cast<uint8_t>(color.r * 255.0f),
					static_cast<uint8_t>(color.g * 255.0f),
					static_cast<uint8_t>(color.b * 255.0f)
				);
			}

			// RGBA32 conversions
			inline RGBA32 ConvertToRGBA32(const RGBA8& color) {
				return RGBA32(
					color.r / 255.0f,
					color.g / 255.0f,
					color.b / 255.0f,
					color.a / 255.0f
				);
			}

			inline RGBA32 ConvertToRGBA32(const RGB8& color) {
				return RGBA32(
					color.r / 255.0f,
					color.g / 255.0f,
					color.b / 255.0f,
					1.0f
				);
			}

			inline RGBA32 ConvertToRGBA32(const RGB32& color) {
				return RGBA32(color.r, color.g, color.b, 1.0f);
			}

			// RGB32 conversions
			inline RGB32 ConvertToRGB32(const RGBA8& color) {
				return RGB32(
					color.r / 255.0f,
					color.g / 255.0f,
					color.b / 255.0f
				);
			}

			inline RGB32 ConvertToRGB32(const RGBA32& color) {
				return RGB32(color.r, color.g, color.b);
			}

			inline RGB32 ConvertToRGB32(const RGB8& color) {
				return RGB32(
					color.r / 255.0f,
					color.g / 255.0f,
					color.b / 255.0f
				);
			}
		}
	}
}