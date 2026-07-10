#include <SGF/Core/Platform/WindowHandle.hpp>
#include <SGF/Core/Platform/Input.hpp>
#include <SGF/Core/Platform/File.hpp>
#include <SGF/Core/Debugging/Logger.hpp>
#include <SGF/Core/Debugging/ErrorCodes.hpp>
#include <SGF/Core/Events/Event.hpp>

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

#include <algorithm>

namespace SGF {
    void WindowHandle::Open(const char* title, uint32_t width, uint32_t height, Flags<WindowOptions> flags, const LayerStack* layerStack) {
        SGF_ASSERT(m_BackendHandle == nullptr);
        
        GLFWmonitor* monitor = nullptr;
        if (flags & WindowOptions::FULLSCREEN) {
            monitor = glfwGetPrimaryMonitor();
            const auto mode = glfwGetVideoMode(monitor);
            width = mode->width;
            height = mode->height;
        }
        m_BackendHandle = glfwCreateWindow(width, height, title, monitor, nullptr);
        if (m_BackendHandle == nullptr) {
            Log::Fatal(ERROR_CREATE_WINDOW);
        }
        
        if (flags & WindowOptions::RESIZABLE) {
            glfwSetWindowAttrib((GLFWwindow*)m_BackendHandle, GLFW_RESIZABLE, GLFW_TRUE);
        } 
        if (flags & WindowOptions::BORDERLESS) {
            glfwSetWindowAttrib((GLFWwindow*)m_BackendHandle, GLFW_DECORATED, GLFW_FALSE);
        }

        if (layerStack != nullptr) {
            SetLayerEventCallbacks(layerStack);
		}
		
        SetFocused();
    }
    void WindowHandle::Close() {
        SGF_ASSERT(m_BackendHandle)
		LayerStack* stack = (LayerStack*)glfwGetWindowUserPointer((GLFWwindow*)m_BackendHandle);
        if (stack != nullptr) {
			WindowCloseEvent event(*this);
            stack->OnEvent(event);
        }
        glfwDestroyWindow((GLFWwindow*)m_BackendHandle);
        m_BackendHandle = nullptr;
    }
    bool WindowHandle::ShouldClose() const {
        SGF_ASSERT(m_BackendHandle);
        return glfwWindowShouldClose((GLFWwindow*)m_BackendHandle);
    }
    uint32_t WindowHandle::GetWidth() const {
        SGF_ASSERT(m_BackendHandle);
        int width;
        glfwGetFramebufferSize((GLFWwindow*)m_BackendHandle, &width, nullptr);
        return width;
    }
    uint32_t WindowHandle::GetHeight() const {
        SGF_ASSERT(m_BackendHandle);
        int height;
        glfwGetFramebufferSize((GLFWwindow*)m_BackendHandle, nullptr, &height);
        return height;
    }
    glm::uvec2 WindowHandle::GetSize() const {
        SGF_ASSERT(m_BackendHandle);
        static_assert(sizeof(int) == sizeof(uint32_t));
        glm::uvec2 size;
        glfwGetFramebufferSize((GLFWwindow*)m_BackendHandle, (int*)&size.x, (int*)&size.y);
        return size;
    }
    bool WindowHandle::IsKeyPressed(Key key) const {
        SGF_ASSERT(m_BackendHandle);
        return glfwGetKey((GLFWwindow*)m_BackendHandle, (int)key) == GLFW_PRESS;
    }
    bool WindowHandle::IsMouseButtonPressed(MouseButton button) const {
        SGF_ASSERT(m_BackendHandle);
        return glfwGetMouseButton((GLFWwindow*)m_BackendHandle, (int)button) == GLFW_PRESS;
    }
    glm::dvec2 WindowHandle::GetCursorPos() const  {
        SGF_ASSERT(m_BackendHandle);
        glm::dvec2 pos;
        glfwGetCursorPos((GLFWwindow*)m_BackendHandle, &pos.x, &pos.y);
        return pos;
    }
    void WindowHandle::SetCursorPos(double xpos, double ypos) const {
        SGF_ASSERT(m_BackendHandle);
        glfwSetCursorPos((GLFWwindow*) m_BackendHandle, xpos, ypos);
    }
    void WindowHandle::SetCursorPos(const glm::dvec2& pos) const {
        SGF_ASSERT(m_BackendHandle);
        glfwSetCursorPos((GLFWwindow*) m_BackendHandle, pos.x, pos.y);
    }
    void WindowHandle::CaptureCursor() const {
        SGF_ASSERT(m_BackendHandle);
        glfwSetInputMode((GLFWwindow*)m_BackendHandle, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        if (glfwRawMouseMotionSupported()) {
            SGF::Log::Info("raw mouse motion supported!");
            glfwSetInputMode((GLFWwindow*)m_BackendHandle, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
        }
    }
    void WindowHandle::HideCursor() const {
        SGF_ASSERT(m_BackendHandle);
        glfwSetInputMode((GLFWwindow*)m_BackendHandle, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
    }
    void WindowHandle::RestrictCursor() const {
        SGF_ASSERT(m_BackendHandle);
        glfwSetInputMode((GLFWwindow*)m_BackendHandle, GLFW_CURSOR, GLFW_CURSOR_CAPTURED);
    }
    void WindowHandle::FreeCursor() const {
        SGF_ASSERT(m_BackendHandle);
        glfwSetInputMode((GLFWwindow*)m_BackendHandle, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
    void WindowHandle::SetCursor(const Cursor& cursor) const {
        SGF_ASSERT(m_BackendHandle);
        glfwSetCursor((GLFWwindow*)m_BackendHandle, (GLFWcursor*)cursor.GetHandle());
    }
    bool WindowHandle::IsFullscreen() const {
        SGF_ASSERT(m_BackendHandle);
        return glfwGetWindowMonitor((GLFWwindow*)m_BackendHandle) != nullptr;
    }
    bool WindowHandle::IsMinimized() const {
        SGF_ASSERT(m_BackendHandle);
        auto size = GetSize();
        return size.x == 0 || size.y == 0;
    }
    bool WindowHandle::IsFocused() const {
        SGF_ASSERT(m_BackendHandle);
        return glfwGetWindowAttrib((GLFWwindow*)m_BackendHandle, GLFW_FOCUSED);
    }
    void WindowHandle::SetLayerEventCallbacks(const LayerStack* layerStack) const {
		SGF_ASSERT(glfwGetWindowUserPointer((GLFWwindow*)m_BackendHandle) == nullptr && "window user pointer already set! cannot set layer event callbacks!");
        glfwSetWindowUserPointer((GLFWwindow*)m_BackendHandle, (void*)layerStack);
        glfwSetKeyCallback((GLFWwindow*)m_BackendHandle, [](GLFWwindow* window, int key, int scancode, int action, int mods) {
			WindowHandle& win = *(WindowHandle*)&window;
			LayerStack* layerStack = (LayerStack*)glfwGetWindowUserPointer((GLFWwindow*)window);
			SGF_ASSERT(layerStack != nullptr && "window user pointer not set to layer stack! cannot set layer event callbacks!");
			switch (action)
			{
			case GLFW_PRESS:
			{
				KeyPressedEvent event(win, (Key)key, mods);
				layerStack->OnEvent(event);
				break;
			}
			case GLFW_RELEASE:
			{
				KeyReleasedEvent event(win, (Key)key, mods);
				layerStack->OnEvent(event);
				break;
			}
			case GLFW_REPEAT:
			{
				KeyRepeatEvent event(win, (Key)key, mods);
				layerStack->OnEvent(event);
				break;
			}
			}
		});
        glfwSetCharCallback((GLFWwindow*)m_BackendHandle, [](GLFWwindow* window, unsigned int codepoint)
        {
            WindowHandle& win = *(WindowHandle*)&window;
			LayerStack* layerStack = (LayerStack*)glfwGetWindowUserPointer((GLFWwindow*)window);
			SGF_ASSERT(layerStack != nullptr && "window user pointer not set to layer stack! cannot set layer event callbacks!");

            KeyTypedEvent event(win, codepoint);
            layerStack->OnEvent(event);
        });

        glfwSetMouseButtonCallback((GLFWwindow*)m_BackendHandle, [](GLFWwindow* window, int button, int action, int mods)
        {
            WindowHandle& win = *(WindowHandle*)&window;
			LayerStack* layerStack = (LayerStack*)glfwGetWindowUserPointer((GLFWwindow*)window);
			SGF_ASSERT(layerStack != nullptr && "window user pointer not set to layer stack! cannot set layer event callbacks!");

            switch (action)
            {
            case GLFW_PRESS:
            {
                MousePressedEvent event(win, (MouseButton)button);
                layerStack->OnEvent(event);
                break;
            }
            case GLFW_RELEASE:
            {
                MouseReleasedEvent event(win, (MouseButton)button);
                layerStack->OnEvent(event);
                break;
            }
            }
        });
        glfwSetWindowFocusCallback((GLFWwindow*)m_BackendHandle, [](GLFWwindow* window, int focus) 
        {
            SGF_ASSERT(window != nullptr);
            WindowHandle& win = *(WindowHandle*)&window;
			LayerStack* layerStack = (LayerStack*)glfwGetWindowUserPointer((GLFWwindow*)window);
			SGF_ASSERT(layerStack != nullptr && "window user pointer not set to layer stack! cannot set layer event callbacks!");

            if (focus == GLFW_TRUE) {
                SGF::Log::Info("Window: {} is now focused!", win.GetTitle());
                Input::SetFocusedNativeWindowHandle(window);
            } else if (focus == GLFW_FALSE) {
                SGF::Log::Info("Window: {} lost focus!", win.GetTitle());
                void* focusedWindow = Input::GetFocusedWindow().m_BackendHandle;
                if (focusedWindow == window) {
                    Input::SetFocusedNativeWindowHandle(nullptr);
                }
            }
			layerStack->OnEvent(WindowFocusEvent(win, focus == GLFW_TRUE));
        });
        glfwSetScrollCallback((GLFWwindow*)m_BackendHandle, [](GLFWwindow* window, double xOffset, double yOffset)
        {
            WindowHandle& win = *(WindowHandle*)&window;
			LayerStack* layerStack = (LayerStack*)glfwGetWindowUserPointer((GLFWwindow*)window);
			SGF_ASSERT(layerStack != nullptr && "window user pointer not set to layer stack! cannot set layer event callbacks!");

            MouseScrollEvent event(win, xOffset, yOffset);
            layerStack->OnEvent(event);
        });
        glfwSetCursorPosCallback((GLFWwindow*)m_BackendHandle, [](GLFWwindow* window, double xPos, double yPos)
        {
            WindowHandle& win = *(WindowHandle*)&window;
			LayerStack* layerStack = (LayerStack*)glfwGetWindowUserPointer((GLFWwindow*)window);
			SGF_ASSERT(layerStack != nullptr && "window user pointer not set to layer stack! cannot set layer event callbacks!");

            MouseMovedEvent event(win, xPos, yPos);
            layerStack->OnEvent(event);
        });
        glfwSetWindowIconifyCallback((GLFWwindow*)m_BackendHandle, [](GLFWwindow* window, int iconified) {
            WindowHandle& win = *(WindowHandle*)&window;
			LayerStack* layerStack = (LayerStack*)glfwGetWindowUserPointer((GLFWwindow*)window);
			SGF_ASSERT(layerStack != nullptr && "window user pointer not set to layer stack! cannot set layer event callbacks!");
            WindowIconifyEvent event(win, iconified);
            layerStack->OnEvent(event);
        });
        glfwSetFramebufferSizeCallback((GLFWwindow*)m_BackendHandle, [](GLFWwindow* window, int width, int height) {
            WindowHandle& win = *(WindowHandle*)&window;
			LayerStack* layerStack = (LayerStack*)glfwGetWindowUserPointer((GLFWwindow*)window);
			SGF_ASSERT(layerStack != nullptr && "window user pointer not set to layer stack! cannot set layer event callbacks!");
            WindowResizeEvent event(win, width, height);
            layerStack->OnEvent(event);
        });
        glfwSetWindowCloseCallback((GLFWwindow*)m_BackendHandle, [](GLFWwindow* window) {
            WindowHandle& win = *(WindowHandle*)&window;
			LayerStack* layerStack = (LayerStack*)glfwGetWindowUserPointer((GLFWwindow*)window);
			SGF_ASSERT(layerStack != nullptr && "window user pointer not set to layer stack! cannot set layer event callbacks!");
			WindowCloseEvent event(win);
            layerStack->OnEvent(event);
        });
    }
    void WindowHandle::SetTitle(const char* title) const {
        SGF_ASSERT(m_BackendHandle);
        glfwSetWindowTitle((GLFWwindow*)m_BackendHandle, title);
    }
    const char* WindowHandle::GetTitle() const {
        SGF_ASSERT(m_BackendHandle);
        return glfwGetWindowTitle((GLFWwindow*)m_BackendHandle);
    }
    void WindowHandle::SetFullscreen() const {
        SGF_ASSERT(m_BackendHandle);
        auto m = glfwGetPrimaryMonitor();
        auto mode = glfwGetVideoMode(m);
        glfwSetWindowMonitor((GLFWwindow*)m_BackendHandle, m, 0, 0, mode->width, mode->height, GLFW_DONT_CARE);
    }
    void WindowHandle::SetWindowed(uint32_t width, uint32_t height) const {
        SGF_ASSERT(m_BackendHandle);
        glfwSetWindowMonitor((GLFWwindow*)m_BackendHandle, nullptr, (int)(width / 2), (int)(height / 2), (int)width, (int)height, GLFW_DONT_CARE);
    }
    void WindowHandle::SetFocused() const {
        if (IsMinimized())
            Restore();
        glfwFocusWindow((GLFWwindow*)m_BackendHandle);
        Input::SetFocusedNativeWindowHandle(m_BackendHandle);
    }
    void WindowHandle::Resize(uint32_t width, uint32_t height) const {
        SGF_ASSERT(m_BackendHandle);
        glfwSetWindowSize((GLFWwindow*)m_BackendHandle, (int)width, (int)height);
    }
    void WindowHandle::Minimize() const {
        SGF_ASSERT(m_BackendHandle);
        glfwIconifyWindow((GLFWwindow*)m_BackendHandle);
    }
    void WindowHandle::Restore() const {
        glfwRestoreWindow((GLFWwindow*)m_BackendHandle);
    }
	std::string WindowHandle::OpenFileDialog(const FileFilter* pFilters, uint32_t filterCount) const {
        SGF_ASSERT(m_BackendHandle);
		NFD_Init();

		nfdu8char_t* outPath;
		nfdopendialogu8args_t args = { 0 };

		std::string filepath;
        if (!NFD_GetNativeWindowFromGLFWWindow((GLFWwindow*)m_BackendHandle, &args.parentWindow)) {
#ifdef SGF_OS_WINDOWS 
			args.parentWindow.handle = (void*)glfwGetWin32Window((GLFWwindow*)m_BackendHandle);
			args.parentWindow.type = NFD_WINDOW_HANDLE_TYPE_WINDOWS;
#elif defined(SGF_OS_LINUX)
#ifdef SGF_USE_X11
            args.parentWindow.handle = (void*)glfwGetX11Window((GLFWwindow*)m_BackendHandle);
            args.parentWindow.type = NFD_WINDOW_HANDLE_TYPE_X11;
#elif defined(SGF_USE_WAYLAND)
            args.parentWindow.handle = (void*)glfwGetWaylandWindow((GLFWwindow*)m_BackendHandle);
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
        SGF_ASSERT(m_BackendHandle);
		NFD_Init();

		nfdu8char_t* outPath;
		nfdsavedialogu8args_t args = {};
		if (!NFD_GetNativeWindowFromGLFWWindow((GLFWwindow*)m_BackendHandle, &args.parentWindow)) {
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
