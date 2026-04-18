#include "WindowHandle.hpp"

#include "SGF_Core.hpp"
#include "Window.hpp"
#include "Render/Device.hpp"
#include "Layers/LayerStack.hpp"
#include "Render/CommandList.hpp"
#include "Render/RenderPass.hpp"
#include "Filesystem/File.hpp"

#ifdef SGF_OS_WINDOWS 
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#define GLFW_EXPOSE_NATIVE_WIN32
#elif defined(SGF_OS_LINUX)
#ifdef SGF_USE_X11
#define GLFW_EXPOSE_NATIVE_X11
#elif defined(SGF_USE_WAYLAND)
#define GLFW_EXPOSE_NATIVE_WAYLAND
#endif
#elif defined(SGF_OS_APPLE)
#define GLFW_EXPOSE_NATIVE_COCOA
#endif
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <nfd.h>
#include <nfd_glfw3.h>

#include "Events/Event.hpp"
#include <algorithm>
#include "Input/Input.hpp"

namespace SGF {
    void WindowHandle::Open(const char* title, uint32_t width, uint32_t height, WindowCreateFlags flags) {
        assert(nativeHandle == nullptr);
        
        GLFWmonitor* monitor = nullptr;
        if (flags & WINDOW_FLAG_FULLSCREEN) {
            monitor = glfwGetPrimaryMonitor();
            const auto mode = glfwGetVideoMode(monitor);
            width = mode->width;
            height = mode->height;
        }
        nativeHandle = glfwCreateWindow(width, height, title, monitor, nullptr);
        if (nativeHandle == nullptr) {
            SGF::Log::Fatal(ERROR_CREATE_WINDOW);
        }
        if (glfwRawMouseMotionSupported()) {
            SGF::Log::Info("raw mouse motion supported!");
            glfwSetInputMode((GLFWwindow*)nativeHandle, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
        }
        if (flags & WINDOW_FLAG_RESIZABLE) {
            glfwSetWindowAttrib((GLFWwindow*)nativeHandle, GLFW_RESIZABLE, GLFW_TRUE);
        } 
        if (flags & WINDOW_FLAG_BORDERLESS) {
            glfwSetWindowAttrib((GLFWwindow*)nativeHandle, GLFW_DECORATED, GLFW_FALSE);
        }
		glfwSetKeyCallback((GLFWwindow*)nativeHandle, [](GLFWwindow* window, int key, int scancode, int action, int mods)
		{
			WindowHandle& win = *(WindowHandle*)&window;
			switch (action)
			{
			case GLFW_PRESS:
			{
				KeyPressedEvent event(win, (Keycode)key, mods);
                LayerStack::Get().OnEvent(event);
				break;
			}
			case GLFW_RELEASE:
			{
				KeyReleasedEvent event(win, (Keycode)key, mods);
                LayerStack::Get().OnEvent(event);
				break;
			}
			case GLFW_REPEAT:
			{
				KeyRepeatEvent event(win, (Keycode)key, mods);
                LayerStack::Get().OnEvent(event);
				break;
			}
			}
		});

        glfwSetCharCallback((GLFWwindow*)nativeHandle, [](GLFWwindow* window, unsigned int codepoint)
        {
            WindowHandle& win = *(WindowHandle*)&window;

            KeyTypedEvent event(win, codepoint);
            LayerStack::Get().OnEvent(event);
            //WindowEvents.dispatch(event);
        });

        glfwSetMouseButtonCallback((GLFWwindow*)nativeHandle, [](GLFWwindow* window, int button, int action, int mods)
        {
            WindowHandle& win = *(WindowHandle*)&window;

            switch (action)
            {
            case GLFW_PRESS:
            {
                MousePressedEvent event(win, (Mousecode)button);
                LayerStack::Get().OnEvent(event);
                //WindowEvents.dispatch(event);
                break;
            }
            case GLFW_RELEASE:
            {
                MouseReleasedEvent event(win, (Mousecode)button);
                LayerStack::Get().OnEvent(event);
                break;
            }
            }
        });
        glfwSetWindowFocusCallback((GLFWwindow*)nativeHandle, [](GLFWwindow* window, int focus) 
        {
            assert(window != nullptr);
            WindowHandle& win = *(WindowHandle*)&window;
            if (focus == GLFW_TRUE) {
                SGF::Log::Info("Window: {} is now focused!", win.GetTitle());
                Input::SetFocusedNativeWindowHandle(window);
            } else if (focus == GLFW_FALSE) {
                SGF::Log::Info("Window: {} lost focus!", win.GetTitle());
                void* focusedWindow = Input::GetFocusedWindow().GetHandle();
                if (focusedWindow == window) {
                    Input::SetFocusedNativeWindowHandle(nullptr);
                }
            }
        });

        glfwSetScrollCallback((GLFWwindow*)nativeHandle, [](GLFWwindow* window, double xOffset, double yOffset)
        {
            WindowHandle& win = *(WindowHandle*)&window;

            MouseScrollEvent event(win, xOffset, yOffset);
            LayerStack::Get().OnEvent(event);
        });
        glfwSetCursorPosCallback((GLFWwindow*)nativeHandle, [](GLFWwindow* window, double xPos, double yPos)
        {
            WindowHandle& win = *(WindowHandle*)&window;

            MouseMovedEvent event(win, xPos, yPos);
            LayerStack::Get().OnEvent(event);
        });
        glfwSetWindowIconifyCallback((GLFWwindow*)nativeHandle, [](GLFWwindow* window, int iconified) {
            WindowHandle& win = *(WindowHandle*)&window;
            WindowIconifyEvent event(win, iconified);
            LayerStack::Get().OnEvent(event);
        });
        glfwSetWindowSizeCallback((GLFWwindow*)nativeHandle, [](GLFWwindow* window, int width, int height) {
            WindowHandle& win = *(WindowHandle*)&window;
            auto size = win.GetSize();
            WindowResizeEvent event(win, size.x, size.y);
            LayerStack::Get().OnEvent(event);
        });
        glfwSetWindowCloseCallback((GLFWwindow*)nativeHandle, [](GLFWwindow* window) {
            WindowHandle& win = *(WindowHandle*)&window;
            SGF::Log::Info("window should be closed: {}", win.GetTitle());
            //WindowIconifyEvent event(win, iconified);
            //LayerStack::Get().OnEvent(event);
        });
        //glfwRestoreWindow()
        SetFocused();
    }
    void WindowHandle::Close() {
        WindowCloseEvent event(*this);
        EventManager::Dispatch(event);
        glfwDestroyWindow((GLFWwindow*)nativeHandle);
        nativeHandle = nullptr;
    }
    bool WindowHandle::ShouldClose() const {
        assert(nativeHandle);
        return glfwWindowShouldClose((GLFWwindow*)nativeHandle);
    }
    uint32_t WindowHandle::GetWidth() const {
        assert(nativeHandle);
        int width;
        glfwGetFramebufferSize((GLFWwindow*)nativeHandle, &width, nullptr);
        return width;
    }
    uint32_t WindowHandle::GetHeight() const {
        assert(nativeHandle);
        int height;
        glfwGetFramebufferSize((GLFWwindow*)nativeHandle, nullptr, &height);
        return height;
    }
    glm::uvec2 WindowHandle::GetSize() const {
        assert(nativeHandle);
        static_assert(sizeof(int) == sizeof(uint32_t));
        glm::uvec2 size;
        glfwGetFramebufferSize((GLFWwindow*)nativeHandle, (int*)&size.x, (int*)&size.y);
        return size;
    }
    bool WindowHandle::IsKeyPressed(Keycode key) const {
        assert(nativeHandle);
        return glfwGetKey((GLFWwindow*)nativeHandle, key) == GLFW_PRESS;
    }
    bool WindowHandle::IsMouseButtonPressed(Mousecode button) const {
        assert(nativeHandle);
        return glfwGetMouseButton((GLFWwindow*)nativeHandle, button) == GLFW_PRESS;
    }
    glm::dvec2 WindowHandle::GetCursorPos() const  {
        assert(nativeHandle);
        glm::dvec2 pos;
        glfwGetCursorPos((GLFWwindow*)nativeHandle, &pos.x, &pos.y);
        return pos;
    }
    void WindowHandle::SetCursorPos(double xpos, double ypos) const {
        assert(nativeHandle);
        glfwSetCursorPos((GLFWwindow*) nativeHandle, xpos, ypos);
    }
    void WindowHandle::SetCursorPos(const glm::dvec2& pos) const {
        assert(nativeHandle);
        glfwSetCursorPos((GLFWwindow*) nativeHandle, pos.x, pos.y);
    }
    void WindowHandle::CaptureCursor() const {
        assert(nativeHandle);
        glfwSetInputMode((GLFWwindow*)nativeHandle, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        glfwSetInputMode((GLFWwindow*)nativeHandle, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
    }
    void WindowHandle::HideCursor() const {
        assert(nativeHandle);
        glfwSetInputMode((GLFWwindow*)nativeHandle, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
    }
    void WindowHandle::RestrictCursor() const {
        assert(nativeHandle);
        glfwSetInputMode((GLFWwindow*)nativeHandle, GLFW_CURSOR, GLFW_CURSOR_CAPTURED);
    }
    void WindowHandle::FreeCursor() const {
        assert(nativeHandle);
        glfwSetInputMode((GLFWwindow*)nativeHandle, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
    void WindowHandle::SetCursor(const Cursor& cursor) const {
        assert(nativeHandle);
        glfwSetCursor((GLFWwindow*)nativeHandle, (GLFWcursor*)cursor.GetHandle());
    }
    bool WindowHandle::IsFullscreen() const {
        assert(nativeHandle);
        return glfwGetWindowMonitor((GLFWwindow*)nativeHandle) != nullptr;
    }
    bool WindowHandle::IsMinimized() const {
        assert(nativeHandle);
        auto size = GetSize();
        return size.x == 0 || size.y == 0;
    }
    bool WindowHandle::IsFocused() const {
        assert(nativeHandle);
        return glfwGetWindowAttrib((GLFWwindow*)nativeHandle, GLFW_FOCUSED);
    }
    void WindowHandle::SetUserPointer(void* pUser) const {
        assert(nativeHandle);
        glfwSetWindowUserPointer((GLFWwindow*)nativeHandle, pUser);
    }
    void WindowHandle::SetTitle(const char* title) const {
        assert(nativeHandle);
        glfwSetWindowTitle((GLFWwindow*)nativeHandle, title);
    }
    const char* WindowHandle::GetTitle() const {
        assert(nativeHandle);
        return glfwGetWindowTitle((GLFWwindow*)nativeHandle);
    }
    void WindowHandle::SetFullscreen() const {
        assert(nativeHandle);
        auto m = glfwGetPrimaryMonitor();
        auto mode = glfwGetVideoMode(m);
        glfwSetWindowMonitor((GLFWwindow*)nativeHandle, m, 0, 0, mode->width, mode->height, GLFW_DONT_CARE);
    }
    void WindowHandle::SetWindowed(uint32_t width, uint32_t height) const {
        assert(nativeHandle);
        glfwSetWindowMonitor((GLFWwindow*)nativeHandle, nullptr, (int)(width / 2), (int)(height / 2), (int)width, (int)height, GLFW_DONT_CARE);
    }
    void WindowHandle::SetFocused() const {
        if (IsMinimized())
            Restore();
        glfwFocusWindow((GLFWwindow*)nativeHandle);
        Input::SetFocusedNativeWindowHandle(nativeHandle);
    }
    void WindowHandle::Resize(uint32_t width, uint32_t height) const {
        assert(nativeHandle);
        glfwSetWindowSize((GLFWwindow*)nativeHandle, (int)width, (int)height);
    }
    void WindowHandle::Minimize() const {
        assert(nativeHandle);
        glfwIconifyWindow((GLFWwindow*)nativeHandle);
    }
    void WindowHandle::Restore() const {
        glfwRestoreWindow((GLFWwindow*)nativeHandle);
    }
	std::string WindowHandle::OpenFileDialog(const FileFilter* pFilters, uint32_t filterCount) const {
        assert(nativeHandle);
		NFD_Init();

		nfdu8char_t* outPath;
		nfdopendialogu8args_t args = { 0 };

		std::string filepath;
        if (!NFD_GetNativeWindowFromGLFWWindow((GLFWwindow*)nativeHandle, &args.parentWindow)) {
#ifdef SGF_OS_WINDOWS 
            args.parentWindow.handle = (void*)glfwGetWin32Window((GLFWwindow*)nativeHandle);
            args.parentWindow.type = NFD_WINDOW_HANDLE_TYPE_WINDOWS;
#elif defined(SGF_OS_LINUX)
#ifdef SGF_USE_X11
            args.parentWindow.handle = (void*)glfwGetX11Window((GLFWwindow*)nativeHandle);
            args.parentWindow.type = NFD_WINDOW_HANDLE_TYPE_X11;
#elif defined(SGF_USE_WAYLAND)
            args.parentWindow.handle = (void*)glfwGetWaylandWindow((GLFWwindow*)nativeHandle);
            args.parentWindow.type = NFD_WINDOW_HANDLE_TYPE_WAYLAND;
#endif
#endif
            SGF::Log::Warn("Failed to get native window handle for file dialog parent! File dialog may not work correctly!");
        }
		args.filterList = (const nfdu8filteritem_t*)(pFilters);
		args.filterCount = filterCount;
		nfdresult_t result = NFD_OpenDialogU8_With(&outPath, &args);
		SGF::Log::Debug("File dialog opened with result: {}", (uint32_t)result);

		if (result == NFD_OKAY) {
			filepath = outPath;
			SGF::Log::Info("Picked file: {}", filepath);
			NFD_FreePathU8(outPath);
		}
		else if (result == NFD_CANCEL) {
			SGF::Log::Info("User pressed cancel.");
		}
		else {
			SGF::Log::Error("File dialog error: {}", NFD_GetError());
			SGF::Log::Debug("File dialog error: {}", NFD_GetError());
		}
		NFD_Quit();

		return filepath;
	}
	std::string WindowHandle::SaveFileDialog(const FileFilter* pFilters, uint32_t filterCount) const
	{
        assert(nativeHandle);
		NFD_Init();

		nfdu8char_t* outPath;
		nfdsavedialogu8args_t args = {};
		if (!NFD_GetNativeWindowFromGLFWWindow((GLFWwindow*)nativeHandle, &args.parentWindow)) {
			SGF::Log::Error("{}", ERROR_OPEN_FILE_DIALOG);
		}
		args.filterList = (const nfdu8filteritem_t*)(pFilters);
		args.filterCount = filterCount;
		nfdresult_t result = NFD_SaveDialogU8_With(&outPath, &args);

		std::string filepath;
		if (result == NFD_OKAY)
		{
			filepath = outPath;
			SGF::Log::Info("User picked savefile: {}", filepath);
			NFD_FreePathU8(outPath);
		}
		else if (result == NFD_CANCEL)
		{
			SGF::Log::Info("User pressed cancel.");
		}
		else
		{
			SGF::Log::Error("File dialog error: {}", NFD_GetError());
		}
		NFD_Quit();

		return filepath;
	}
}
