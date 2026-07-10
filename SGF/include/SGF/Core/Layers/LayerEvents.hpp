#pragma once

#include <stdint.h>
#include <SGF/Core/Events/Event.hpp>
#include <SGF/Core/Platform/InputMappings.hpp>
#include <SGF/Core/Math/Math.hpp>
#include <SGF/Core/Flags.hpp>

namespace SGF {
	class WindowHandle;
    class WindowEvent {
	private:
		WindowHandle& m_WindowHandle;
	public:
		inline WindowEvent(WindowHandle& handle) : m_WindowHandle(handle) {};
		inline WindowHandle& GetWindow() const { return m_WindowHandle; }
	};
	class WindowIconifyEvent : public WindowEvent {
	public:
		inline WindowIconifyEvent(WindowHandle& window, bool wasIconified) : WindowEvent(window), iconified(wasIconified) {};
		inline bool Iconified() const { return iconified; }
	private:
		bool iconified;
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
		inline WindowMinimizeEvent(WindowHandle& window, bool wasMinimized) : WindowEvent(window), m_IsMinimized(wasMinimized) {}
		inline bool IsMinimized() const { return m_IsMinimized; }
	private:
		bool m_IsMinimized;
	};
	class WindowFocusEvent : public WindowEvent {
	public:
		inline WindowFocusEvent(WindowHandle& window, bool wasFocused) : WindowEvent(window), m_IsFocused(wasFocused){}
		inline bool IsFocused() const { return m_IsFocused; }
	private:
		bool m_IsFocused;
	};
	class MousePressedEvent : public WindowEvent {
	public:
		inline MousePressedEvent(WindowHandle& window, MouseButton mouseButton) : WindowEvent(window), m_MouseButton(mouseButton) {}
		inline MouseButton GetButton() const { return m_MouseButton; }
	private:
		MouseButton m_MouseButton;
	};
	class MouseReleasedEvent : public WindowEvent {
	public:
		inline MouseReleasedEvent(WindowHandle& window, MouseButton mouseButton) : WindowEvent(window), m_MouseButton(mouseButton) {}
		inline MouseButton GetButton() const { return m_MouseButton; }
	private:
		MouseButton m_MouseButton;
	};
	class MouseScrollEvent : public WindowEvent {
	private:
		glm::dvec2 m_Pos;
	public:
		inline MouseScrollEvent(WindowHandle& window, double xOffset, double yOffset) : WindowEvent(window), m_Pos(xOffset, yOffset) {}
		inline double GetOffsetX() const { return m_Pos.x; }
		inline double GetOffsetY() const { return m_Pos.y; }
		inline const glm::dvec2& GetOffset() const { return m_Pos; }
	};
	class MouseMovedEvent : public WindowEvent {
	public:
		inline MouseMovedEvent(WindowHandle& window, double xpos, double ypos) : WindowEvent(window), m_Pos(xpos, ypos) {}
		inline double GetX() const { return m_Pos.x; }
		inline double GetY() const { return m_Pos.y; }
		inline const glm::dvec2& GetPos() const { return m_Pos; }
	private:
		glm::dvec2 m_Pos;
	};
	class KeyTypedEvent : public WindowEvent {
	public:
		inline KeyTypedEvent(WindowHandle& window, uint32_t codepoint): WindowEvent(window), m_CharCode(codepoint) {}
		inline uint32_t GetChar() const { return m_CharCode; }
	private:
		uint32_t m_CharCode;
	};
	class KeyEvent : public WindowEvent {
	public:
		inline KeyEvent(WindowHandle& window, Key keycode, uint32_t mods) : WindowEvent(window), m_KeyCode(keycode), m_Modifiers(mods) {}
		inline Key GetKey() const { return m_KeyCode; }
		inline Flags<KeyModifier> GetModifiers() const { return m_Modifiers; }
	private:
		Key m_KeyCode;
		Flags<KeyModifier> m_Modifiers;
	};
	class KeyPressedEvent : public KeyEvent {
	public:
		inline KeyPressedEvent(WindowHandle& window, Key keycode, uint32_t mods) : KeyEvent(window, keycode, mods) {}
	};
	class KeyReleasedEvent : public KeyEvent {
	public:
		inline KeyReleasedEvent(WindowHandle& window, Key keycode, uint32_t mods) : KeyEvent(window, keycode, mods) {}
	};
	class KeyRepeatEvent : public KeyEvent {
	public:
		inline KeyRepeatEvent(WindowHandle& window, Key keycode, uint32_t mods) : KeyEvent(window, keycode, mods) {}
	};
	class RenderEvent {
    public:
        inline RenderEvent(double dTime, glm::uvec2 framebufferSize) 
            : deltaTime(dTime), renderArea(framebufferSize) {}
        inline double GetTime() const { return deltaTime; }
        inline glm::uvec2 GetFramebufferSize() const { return renderArea; }
    private:
        double deltaTime;
        const glm::uvec2 renderArea;
    };
    class UpdateEvent {
    public:
        inline UpdateEvent(double dTime) : deltaTime(dTime) {}
        inline double GetDeltaTime() const { return deltaTime; }
    private:
        double deltaTime;
    };
} // namespace SGF
