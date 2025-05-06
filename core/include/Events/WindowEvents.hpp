#pragma once

#include "SGF_Core.hpp"
#include "Events/Event.hpp"

namespace SGF {
	class Event {};
	class WindowEvent : Event {
	public:
		inline WindowEvent(Window& window) : windowHandle(window) {};
		inline Window& getWindow() const { return windowHandle; }
	private:
		Window& windowHandle;
	};
	class WindowResizeEvent : public WindowEvent {
	public:
		inline WindowResizeEvent(Window& window, uint32_t newWidth, uint32_t newHeight) : WindowEvent(window), width(newWidth), height(newHeight) {}
		inline uint32_t getWidth() const { return width; }
		inline uint32_t getHeight() const { return height; }
	private:
		uint32_t width;
		uint32_t height;
	};
	class WindowCloseEvent : public WindowEvent {
	public:
		inline WindowCloseEvent(Window& window) : WindowEvent(window) {}
	};
	class WindowMinimizeEvent : WindowEvent {
	public:
		inline WindowMinimizeEvent(Window& window) : WindowEvent(window) {}
	};

	class InputEvent : public WindowEvent {
	public:	
		inline InputEvent(Window& window) : WindowEvent(window), handled(false) {}
		inline void setHandled() { handled = true; }
		inline bool isHandle() { return handled; }
	private:
		bool handled = false;
	};

	class MousePressedEvent : public InputEvent {
	public:
		inline MousePressedEvent(Window& window, uint32_t mousecode) : InputEvent(window), mouseCode(mousecode) {}
	private:
		uint32_t mouseCode;
	};
	class MouseReleasedEvent : public InputEvent {
	public:
		inline MouseReleasedEvent(Window& window, uint32_t mousecode) : InputEvent(window), mouseCode(mousecode) {}
	private:
		uint32_t mouseCode;
	};
	class MouseScrollEvent : public InputEvent {
	public:
		inline MouseScrollEvent(Window& window, double xOffset, double yOffset) : InputEvent(window), x(xOffset), y(yOffset) {}
		inline double xOffset() { return x; }
		inline double yOffset() { return y; }
	private:
		double x;
		double y;
	};
	class MouseMovedEvent : public InputEvent {
	public:
		inline MouseMovedEvent(Window& window, double xpos, double ypos) : InputEvent(window), xPos(xpos), yPos(ypos) {}
		inline double getX() { return xPos; }
		inline double getY() { return yPos; }
	private:
		double xPos;
		double yPos;
	};

	class KeyTypedEvent : public InputEvent {
	public:
		inline KeyTypedEvent(Window& window, uint32_t codepoint): InputEvent(window), charCode(codepoint) {}
		inline uint32_t getChar() { return charCode; }
	private:
		uint32_t charCode;
	};

	class KeyEvent : public InputEvent {
	public:
		inline KeyEvent(Window& window, uint32_t keycode, uint32_t mods) : InputEvent(window), keyCode(keycode), modifier(mods) {}
		inline uint32_t getKey() { return keyCode; }
		inline uint32_t getMod() { return modifier; }
	private:
		uint32_t keyCode;
		uint32_t modifier;
	};
	class KeyPressedEvent : public KeyEvent {
	public:
		inline KeyPressedEvent(Window& window, uint32_t keycode, uint32_t mods) : KeyEvent(window, keycode, mods) {}
	};
	class KeyReleasedEvent : public KeyEvent {
	public:
		inline KeyReleasedEvent(Window& window, uint32_t keycode, uint32_t mods) : KeyEvent(window, keycode, mods) {}
	};
	class KeyRepeatEvent : public KeyEvent {
	public:
		inline KeyRepeatEvent(Window& window, uint32_t keycode, uint32_t mods) : KeyEvent(window, keycode, mods) {}
	};
}