#pragma once

#include "SGF_Core.hpp"
#include "Keycodes.hpp"
#include "Mousecodes.hpp"

namespace SGF {
	Window* getCurrentWindow();

	bool isKeyPressed(Keycode key);
	bool isMousePressed(Mousecode button);
	glm::vec2 getCursorPos();
}