#pragma once

#include "SGF_Core.hpp"
#include "Input/Keycodes.hpp"
#include "Input/Mousecodes.hpp"
#include "Render/Display.hpp"

namespace SGF {
    struct FileFilter {
		const char* filterDescription;
		const char* filters;
	};
    class Window {
    public:
        //inline static Window* getFocusedWindow() { return focusedWindow; }
        inline static Window* GetFocused() { return FocusedWindow; }
        static VkExtent2D getMonitorSize(uint32_t index);
        static VkExtent2D getMaxMonitorSize();
        static uint32_t getMonitorCount();

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
        ~Window();
        Window(Window&& other) noexcept;
        Window(const Window& other) = delete;
        Window(Window& other) = delete;
        
        void open(const char* name, uint32_t width, uint32_t height, WindowCreateFlags flags = FLAG_NONE);
        void close();

        inline void setRenderPass(const VkAttachmentDescription* pAttachments, uint32_t attCount, const VkSubpassDescription* pSubpasses, uint32_t subpassCount, const VkSubpassDependency* pDependencies, uint32_t dependencyCount) 
        { display.updateRenderPass(surface, width, height, pAttachments, attCount, pSubpasses, subpassCount, pDependencies, dependencyCount); }
        inline void setRenderPass(const std::vector<VkAttachmentDescription>& attachments, const std::vector<VkSubpassDescription>& subpasses, const std::vector<VkSubpassDependency>& dependencies)
        { display.updateRenderPass(surface, width, height, attachments.data(), (uint32_t)attachments.size(), subpasses.data(), (uint32_t)subpasses.size(), dependencies.data(), (uint32_t)dependencies.size()); }

        inline void resizeFramebuffers(uint32_t w, uint32_t h) { width = w; height = h; display.updateFramebuffers(surface, w, h); }

		inline void requestSurfaceFormat(VkSurfaceFormatKHR requested) { display.requestSurfaceFormat(requested); }
		inline void requestColorSpace(VkColorSpaceKHR requested) { display.requestColorSpace(requested); }
		inline void requestImageFormat(VkFormat requested) { display.requestImageFormat(requested); }
		inline void requestPresentMode(VkPresentModeKHR requested) { display.requestPresentMode(requested); }

        void nextFrame(VkSemaphore imageAvailable, VkFence fence = VK_NULL_HANDLE);
        inline void presentFrame(VkSemaphore waitSemaphore) { display.presentFrame(waitSemaphore); }
        inline void presentFrame(const VkSemaphore* pWaitSemaphores, uint32_t waitCount) { display.presentFrame(pWaitSemaphores, waitCount); }
        inline void presentFrame(const std::vector<VkSemaphore>& waitSemaphores) { display.presentFrame(waitSemaphores.data(), (uint32_t)waitSemaphores.size()); }
        //====================================================================
        //========================Window Information==========================
        //====================================================================
        bool isOpen() const { return window != nullptr; }
        bool shouldClose() const ;
        inline uint32_t getWidth() const { return width; }
        inline uint32_t getHeight() const { return height; }
        inline VkExtent2D getFramebufferSize() const { return { width, height }; }
        inline VkSurfaceKHR getSurface() const { return surface; }
        inline VkRenderPass getRenderPass() const { return display.getRenderPass(); }
        inline VkFramebuffer getCurrentFramebuffer() const { return display.getCurrentFramebuffer(); }
        inline uint32_t getImageIndex() const { return display.getImageIndex(); }
        inline uint32_t getImageCount() const { return display.getImageCount(); }
        inline operator VkSurfaceKHR() const { return surface; }
        inline const void* getNativeWindow() const { return window; }
        const char* getName() const;
        //====================================================================
        //========================Window Modifiers============================
        //====================================================================
        void onUpdate();
        bool isFullscreen() const;
        bool isMinimized();
        void enableVsync();
        void disableVsync();
        void setFullscreen();
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
        Window(const char* name, uint32_t width, uint32_t height, WindowCreateFlags flags = FLAG_NONE);
    private:
        inline Window() {}
        friend Device;
        uint32_t width = 0;
        uint32_t height = 0;
        void* window = nullptr;
        VkSurfaceKHR surface = VK_NULL_HANDLE;
        Display display;
        inline static Window* FocusedWindow = nullptr;
    };
}