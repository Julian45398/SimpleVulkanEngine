#pragma once

#include "SGF_Core.hpp"

#include "Input/Keycodes.hpp"
#include "Input/Mousecodes.hpp"


namespace SGF {
    class WindowEvent {
	public:
		inline WindowEvent(WindowHandle& handle) : windowHandle(handle) {};
		inline WindowHandle& GetWindow() const { return windowHandle; }
	private:
		WindowHandle& windowHandle;
	};
	class WindowIconifyEvent : public WindowEvent {
	public:
		inline WindowIconifyEvent(WindowHandle& window, int wasIconified) : WindowEvent(window), iconified(wasIconified) {};
		inline bool Iconified() const { return iconified; }
	private:
		int iconified;
	};
	class WindowResizeEvent : public WindowEvent {
	public:
		inline WindowResizeEvent(WindowHandle& window, uint32_t newWidth, uint32_t newHeight) : WindowEvent(window), width(newWidth), height(newHeight) {}
		inline uint32_t GetWidth() const { return width; }
		inline uint32_t GetHeight() const { return height; }
	private:
		uint32_t width;
		uint32_t height;
	};
	class WindowCloseEvent : public WindowEvent {
	public:
		inline WindowCloseEvent(WindowHandle& window) : WindowEvent(window) {}
	};
	class WindowOpenEvent : public WindowEvent {
	public:
		inline WindowOpenEvent(WindowHandle& window) : WindowEvent(window) {}
	};
	class WindowMinimizeEvent : public WindowEvent {
	public:
		inline WindowMinimizeEvent(WindowHandle& window, bool wasMinimized) : WindowEvent(window) {}
		inline bool WasMinimized() const { return minimized; }
	private:
		bool minimized;
	};
	class MousePressedEvent : public WindowEvent {
	public:
		inline MousePressedEvent(WindowHandle& window, Mousecode mousecode) : WindowEvent(window), mouseCode(mousecode) {}
		inline Mousecode GetButton() const { return mouseCode; }
	private:
		Mousecode mouseCode;
	};
	class MouseReleasedEvent : public WindowEvent {
	public:
		inline MouseReleasedEvent(WindowHandle& window, Mousecode mousecode) : WindowEvent(window), mouseCode(mousecode) {}
		inline Mousecode GetButton() const { return mouseCode; }
	private:
		Mousecode mouseCode;
	};
	class MouseScrollEvent : public WindowEvent {
	public:
		inline MouseScrollEvent(WindowHandle& window, double xOffset, double yOffset) : WindowEvent(window), pos(xOffset, yOffset) {}
		inline double GetOffsetX() const { return pos.x; }
		inline double GetOffsetY() const { return pos.y; }
		inline const glm::dvec2& GetOffset() const { return pos; }
	private:
		glm::dvec2 pos;
	};
	class MouseMovedEvent : public WindowEvent {
	public:
		inline MouseMovedEvent(WindowHandle& window, double xpos, double ypos) : WindowEvent(window), pos(xpos, ypos) {}
		inline double GetX() const { return pos.x; }
		inline double GetY() const { return pos.y; }
		inline const glm::dvec2& GetPos() const { return pos; }
	private:
		glm::dvec2 pos;
	};
	class KeyTypedEvent : public WindowEvent {
	public:
		inline KeyTypedEvent(WindowHandle& window, uint32_t codepoint): WindowEvent(window), charCode(codepoint) {}
		inline uint32_t GetChar() const { return charCode; }
	private:
		uint32_t charCode;
	};
	class KeyEvent : public WindowEvent {
	public:
		inline KeyEvent(WindowHandle& window, Keycode keycode, uint32_t mods) : WindowEvent(window), keyCode(keycode), modifier(mods) {}
		inline Keycode GetKey() const { return keyCode; }
		inline uint32_t GetMod() const { return modifier; }
	private:
		Keycode keyCode;
		uint32_t modifier;
	};
	class KeyPressedEvent : public KeyEvent {
	public:
		inline KeyPressedEvent(WindowHandle& window, Keycode keycode, uint32_t mods) : KeyEvent(window, keycode, mods) {}
	};
	class KeyReleasedEvent : public KeyEvent {
	public:
		inline KeyReleasedEvent(WindowHandle& window, Keycode keycode, uint32_t mods) : KeyEvent(window, keycode, mods) {}
	};
	class KeyRepeatEvent : public KeyEvent {
	public:
		inline KeyRepeatEvent(WindowHandle& window, Keycode keycode, uint32_t mods) : KeyEvent(window, keycode, mods) {}
	};
    class DeviceEvent {
    public:
        inline DeviceEvent(Device& dev) : device(dev) {}
        inline const Device& GetDevice() const { return device; }
    private:
        Device& device;
    };
    class DeviceDestroyEvent : public DeviceEvent {
    public:
        inline DeviceDestroyEvent(Device& dev) : DeviceEvent(dev) {}
    };
    class DeviceCreateEvent : public DeviceEvent {
    public:
        inline DeviceCreateEvent(Device& dev) : DeviceEvent(dev) {}
    };
} // namespace SGF
