#include "Input/Input.hpp"

#include <glfw/glfw3.h>


namespace SGF {
	bool isKeyPressed(Keycode key) {
		return false;
	}
	bool isMousePressed(Mousecode button) {
		return false;
	}
	glm::vec2 getCursorPos() {
		return glm::vec2(0.f, 0.f);
	}
}