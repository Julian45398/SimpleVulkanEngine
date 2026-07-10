#pragma once

#include "WindowHandle.hpp"
#include "Cursor.hpp"
#include "InputMappings.hpp"

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
        bool IsMouseButtonPressed(MouseButton button);
        bool IsKeyPressed(Key key);
        WindowHandle& GetFocusedWindow();
    };
}