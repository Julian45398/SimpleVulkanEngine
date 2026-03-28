#include "Input.hpp"
#include "WindowHandle.hpp"


#include <glfw/glfw3.h>

namespace SGF {
    namespace Input {
	    WindowHandle s_FocusedWindow;
    }
    void Input::PollEvents() {
        glfwPollEvents();
    }
    void Input::WaitEvents() {
        glfwWaitEvents();
    }
    glm::dvec2 Input::GetCursorPos() {
        static glm::dvec2 pos(0, 0);
        if (!HasFocus()) {
            SGF::Log::Warn("Requesting cursor position but Window is not focused!");
            return glm::dvec2(0, 0);
        }
        assert(HasFocus());
        //assert(s_FocusedWindow.IsFocused());
        pos = s_FocusedWindow.GetCursorPos();
        return pos;
    }
    void Input::SetCursorPos(double xpos, double ypos) {
        assert(HasFocus());
        //assert(s_FocusedWindow.IsFocused());
        s_FocusedWindow.SetCursorPos(xpos, ypos);
    }
    void Input::SetCursorPos(const glm::dvec2& pos) {
        assert(HasFocus());
        //assert(s_FocusedWindow.IsFocused());
        s_FocusedWindow.SetCursorPos(pos);
    }
    void Input::SetFocusedNativeWindowHandle(void* nativeWindowHandle) {
        s_FocusedWindow.SetHandle(nativeWindowHandle);
    }
    void Input::CaptureCursor() {
        assert(HasFocus());
        //assert(s_FocusedWindow.IsFocused());
        s_FocusedWindow.CaptureCursor();
    }
    void Input::HideCursor() {
        assert(HasFocus());
        //assert(s_FocusedWindow.IsFocused());
        s_FocusedWindow.HideCursor();
    }
    void Input::RestrictCursor() {
        assert(HasFocus());
        //assert(s_FocusedWindow.IsFocused());
        s_FocusedWindow.RestrictCursor();
    }
    void Input::FreeCursor() {
        assert(HasFocus());
        //assert(s_FocusedWindow.IsFocused());
        s_FocusedWindow.FreeCursor();
    }
    void Input::SetCursor(const Cursor& cursor) {
        assert(HasFocus());
        //assert(s_FocusedWindow.IsFocused());
        s_FocusedWindow.SetCursor(cursor);
    }
    bool Input::IsMouseButtonPressed(Mousecode button) {
        assert(HasFocus());
        //assert(s_FocusedWindow.IsFocused());
        return HasFocus() && GetFocusedWindow().IsMouseButtonPressed(button);
    }
    bool Input::IsKeyPressed(Keycode key) {
        assert(HasFocus());
        //assert(s_FocusedWindow.IsFocused());
        return HasFocus() && GetFocusedWindow().IsKeyPressed(key);
    }
    bool Input::HasFocus() {
        return s_FocusedWindow.IsOpen();
    }
    WindowHandle& Input::GetFocusedWindow() { return s_FocusedWindow; }
}