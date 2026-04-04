#pragma once

#include "WindowHandle.hpp"
#include "Cursor.hpp"
#include "Keycodes.hpp"
#include "Mousecodes.hpp"

namespace SGF {
    namespace Input {
        void PollEvents();
        void WaitEvents();
        bool HasFocus();
        glm::dvec2 GetCursorPos();
        void SetCursorPos(double xpos, double ypos);
        void SetCursorPos(const glm::dvec2& pos);
        void CaptureCursor();
        void HideCursor();
        void RestrictCursor();
        void FreeCursor();
        void SetCursor(const Cursor& cursor);
        void SetFocusedNativeWindowHandle(void* nativeWindowHandle);
        bool IsMouseButtonPressed(Mousecode button);
        bool IsKeyPressed(Keycode key);
        WindowHandle& GetFocusedWindow();
    };
}