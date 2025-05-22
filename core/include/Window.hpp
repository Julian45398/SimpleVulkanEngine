#pragma once

#include "SGF_Core.hpp"
#include "Input/Keycodes.hpp"
#include "Input/Mousecodes.hpp"

namespace SGF {
    enum WindowCreateFlagBits {
        WINDOW_FLAG_FULLSCREEN = BIT(0),
        WINDOW_FLAG_RESIZABLE = BIT(1),
        WINDOW_FLAG_MINIMIZED = BIT(2),
        WINDOW_FLAG_MAXIMIZED = BIT(3)
    };
    typedef Flags WindowCreateFlags;

	struct FileFilter {
		const char* filterDescription;
		const char* filters;
	};
    //inline WindowEventManager WindowEvents;

    class Window {
    public:
        inline static Window* getFocusedWindow() { return focusedWindow; }
        static VkExtent2D Window::getMonitorSize(uint32_t index);
        static VkExtent2D Window::getMaxMonitorSize();
        static uint32_t Window::getMonitorCount();


        //====================================================================
        //========================Window Creation=============================
        //====================================================================
        /**
         * @brief Creates a window with the specified name, width, and height.
         * 
         * Creates a window without binding it to a GPU. 
         * When the window is created in fullscreen mode the width and the height specify 
         * the target resolution.
         * 
         * @param name - the name of the window
         * @param width - window width - or when fullscreen the resolution
         * @param height - window height - or when fullscreen the resolution
         * @param fullscreen - controls whether the window should be created in fullscreen mode
         */
        Window(const char* name, uint32_t width, uint32_t height, WindowCreateFlags flags = FLAG_NONE);
        ~Window();
        Window(Window&& other) noexcept;
        Window(const Window& other) = delete;
        Window(Window& other) = delete;
        
        void open(const char* name, uint32_t width, uint32_t height, WindowCreateFlags flags = FLAG_NONE);
        void close();

        //====================================================================
        //========================Window Information==========================
        //====================================================================
        bool isOpen() const { return window != nullptr; }
        //bool hasDevice() const { return pDevice != nullptr; }
        bool shouldClose() const ;
        inline uint32_t getWidth() const { return width; }
        inline uint32_t getHeight() const { return height; }
        inline VkSurfaceKHR getSurface() const { return surface; }
        inline operator VkSurfaceKHR() const { return surface; }
        //inline VkSurfaceFormatKHR getSurfaceFormat() const { return surfaceFormat; }
        //inline VkFormat getFormat() const { return surfaceFormat.format; }
        //inline VkColorSpaceKHR getColorspace() const { return surfaceFormat.colorSpace; }
        //inline VkPresentModeKHR getPresentMode() const { return presentMode; }
        const char* getName() const;
        //====================================================================
        //========================Window Modifiers============================
        //====================================================================
        //void bindDevice(Device& device);
        //void unbindDevice();
        void onUpdate();
        void nextImage(VkSemaphore signalSemaphore, VkFence fence);
        void presentImage(VkSemaphore waitSemaphore, VkFence fence) const;
        bool isFullscreen() const;
        bool isMinimized();
        void enableVsync();
        void disableVsync();
        void setFullscreen();
        //void setFullscreenKeepResolution();
        void setWindowed(uint32_t width, uint32_t height);
        void minimize();
        //====================================================================
        //============================Window Input============================
        //====================================================================
        bool isKeyPressed(Keycode key) const;
        bool isMousePressed(Mousecode button) const;
        glm::dvec2 getCursorPos() const;

		std::string openFileDialog(const FileFilter& filter) const;
		std::string openFileDialog(uint32_t filterCount, const FileFilter* pFilters) const;
		std::string saveFileDialog(const FileFilter& filter) const;
		std::string saveFileDialog(uint32_t filterCount, const FileFilter* pFilters) const;
    private:
        static void onResize(WindowResizeEvent& event);
        static void onMinimize(WindowMinimizeEvent& event);
        static void onDeviceDestroy(const DeviceDestroyEvent& event, Window* window);
    private:
        friend Device;
        //void createSwapchain();
        //void createResources();
        //void destroyResources();
        uint32_t width = 0;
        uint32_t height = 0;
        void* window = nullptr;
        VkSurfaceKHR surface = VK_NULL_HANDLE;
        inline static Window* focusedWindow = nullptr;
    };
}