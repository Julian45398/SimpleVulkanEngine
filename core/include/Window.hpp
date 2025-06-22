#pragma once

#include "SGF_Core.hpp"
#include "Input/Keycodes.hpp"
#include "Input/Mousecodes.hpp"
#include <string.h>

namespace SGF {
    class Input {
    public:
        static void PollEvents();
        static void WaitEvents();
        static glm::dvec2 GetCursorPos();
        static bool IsMouseButtonPressed(Mousecode button);
        static bool IsKeyPressed(Keycode key);
    };
    struct FileFilter {
		const char* filterDescription;
		const char* filters;
	};
    struct WindowSettings {
        char title[256];
        WindowCreateFlags createFlags;
        uint32_t width;
        uint32_t height;
        VkSampleCountFlagBits sampleCount;
        VkFormat depthFormat;
        VkAttachmentLoadOp colorLoadOp;
    };
    class WindowHandle {
    public:
        void open(const char* title, uint32_t width, uint32_t height, WindowCreateFlags windowFlags);
        void close();
        inline ~WindowHandle() { close(); }
        inline WindowHandle(const char* title, uint32_t width, uint32_t height, WindowCreateFlags windowFlags) 
        { open(title, width, height, windowFlags); }
        inline WindowHandle() : nativeHandle(nullptr) {};

        bool shouldClose() const;
        uint32_t getWidth() const;
        uint32_t getHeight() const;
        VkExtent2D getSize() const;
        bool isKeyPressed(Keycode key) const;
        bool isMouseButtonPressed(Mousecode button) const;
        glm::dvec2 getCursorPos() const;
        void captureCursor() const;
        void freeCursor() const;
        bool isFullscreen() const;
        bool isMinimized() const;
        bool isFocused() const;
        inline bool isOpen() const { return nativeHandle != nullptr; }
        inline void* getHandle() const { return nativeHandle; }

        void setUserPointer(void* pUser) const;
        void setTitle(const char* title) const;
        const char* getTitle() const;
        void setCursorPos(const glm::dvec2& newpos) const;
        void setFullscreen() const;
        void setWindowed(uint32_t width, uint32_t height) const;
        void resize(uint32_t width, uint32_t height) const;
        void minimize() const;
    private:
        void* nativeHandle;
    };
    class Window {
    public:
		inline static const VkSurfaceFormatKHR DEFAULT_SURFACE_FORMAT = { VK_FORMAT_B8G8R8A8_UNORM, VK_COLORSPACE_SRGB_NONLINEAR_KHR };
        static VkAttachmentDescription createSwapchainAttachment(VkAttachmentLoadOp loadOp);
        inline static void* GetNativeFocused() { return s_NativeFocused; }
        //inline static Window* GetFocused() { return FocusedWindow; }
        static VkExtent2D getMonitorSize(uint32_t index);
        static VkExtent2D getMaxMonitorSize();
        static uint32_t getMonitorCount();
        inline static Window& Get() { return s_MainWindow; }
        static void Open() { Get().open(s_WindowSettings.title, s_WindowSettings.width, s_WindowSettings.height, s_WindowSettings.createFlags); }
        inline static void SetCreateFlags(WindowCreateFlags flags) { s_WindowSettings.createFlags = flags; }
        inline static void SetSize(uint32_t width, uint32_t height) { s_WindowSettings.width = width, s_WindowSettings.height = height; }
        inline static void SetTitle(const char* title) { assert(strlen(title) < ARRAY_SIZE(s_WindowSettings.title)); strncpy(s_WindowSettings.title, title, ARRAY_SIZE(s_WindowSettings.title)); }
        inline static void EnableClear(float r, float g, float b, float a);
        inline static void Close() { Get().close(); }

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
         * @param flags - window flags
         * @param multisampleCount - multisampleCount 
         */
        inline Window(const char* name, uint32_t width, uint32_t height, WindowCreateFlags flags = FLAG_NONE, VkSampleCountFlagBits multisampleCount = VK_SAMPLE_COUNT_1_BIT) 
        { open(name, width, height, flags, multisampleCount); }

        inline ~Window() { close(); }

        Window(Window&& other) = delete;
        Window(const Window& other) = delete;
        Window(Window& other) = delete;
        
        void open(const char* name, uint32_t width, uint32_t height, WindowCreateFlags flags = FLAG_NONE, VkSampleCountFlagBits multisampleCount = VK_SAMPLE_COUNT_1_BIT);
        void close();

        void setRenderPass(const VkAttachmentDescription* pAttachments, const VkClearValue* pClearValues, uint32_t attCount, const VkSubpassDescription* pSubpasses, uint32_t subpassCount, const VkSubpassDependency* pDependencies, uint32_t dependencyCount);

        template<size_t ATTACHMENT_COUNT, size_t SUBPASS_COUNT, size_t DEPENDENCY_COUNT>
        inline void setRenderPass(const VkAttachmentDescription(&attachments)[ATTACHMENT_COUNT], const VkClearValue(&clearValues)[ATTACHMENT_COUNT], const VkSubpassDescription(&subpasses)[SUBPASS_COUNT], const VkSubpassDependency(&dependencies)[DEPENDENCY_COUNT])
        { setRenderPass(attachments, clearValues, ATTACHMENT_COUNT, subpasses, SUBPASS_COUNT, dependencies, DEPENDENCY_COUNT); }
        
        inline void setRenderPass(const std::vector<VkAttachmentDescription>& attachments, const std::vector<VkSubpassDescription>& subpasses, const std::vector<VkSubpassDependency>& dependencies, const std::vector<VkClearValue>& clearValues)
        { assert(attachments.size() == clearValues.size()); setRenderPass(attachments.data(), clearValues.data(), (uint32_t)attachments.size(), subpasses.data(), (uint32_t)subpasses.size(), dependencies.data(), (uint32_t)dependencies.size()); }

        inline void resizeFramebuffers(uint32_t w, uint32_t h) { width = w; height = h; updateFramebuffers(); }

		inline void requestSurfaceFormat(VkSurfaceFormatKHR requested) { surfaceFormat = requested; }
		inline void requestColorSpace(VkColorSpaceKHR requested) { surfaceFormat.colorSpace = requested; }
		inline void requestImageFormat(VkFormat requested) { surfaceFormat.format = requested; }
		inline void requestPresentMode(VkPresentModeKHR requested) { presentMode = requested; }

        void nextFrame(VkSemaphore imageAvailableSignal, VkFence fence = VK_NULL_HANDLE);
		void presentFrame(const VkSemaphore* pWaitSemaphores, uint32_t waitCount);
        template<uint32_t WAIT_COUNT>
        inline void presentFrame(const VkSemaphore(&waitSemaphores)[WAIT_COUNT]) { presentFrame(waitSemaphores, WAIT_COUNT); }
        inline void presentFrame(const std::vector<VkSemaphore>& waitSemaphores) { presentFrame(waitSemaphores.data(), (uint32_t)waitSemaphores.size()); }
		inline void presentFrame(VkSemaphore waitSemaphore) { presentFrame(&waitSemaphore, 1); }
        //====================================================================
        //========================Window Information==========================
        //====================================================================
        bool isOpen() const { return windowHandle.isOpen(); }
        bool hasRenderTarget() const { return renderPass != nullptr; }
        bool shouldClose() const;

        inline uint32_t getWidth() const { return width; }
        inline uint32_t getHeight() const { return height; }
        inline VkExtent2D getFramebufferSize() const { return {width, height}; }
        inline VkSurfaceKHR getSurface() const { return surface; }
        inline VkRenderPass getRenderPass() const { return renderPass; }
		inline VkSwapchainKHR getSwapchain() const { return swapchain; }

        inline uint32_t getImageIndex() const { return imageIndex; }
        inline uint32_t getImageCount() const { return imageCount; }
        inline operator VkSurfaceKHR() const { return surface; }
        inline const WindowHandle& getNativeWindow() const { return windowHandle; }
        const char* getName() const;


		inline const VkImage* getSwapchainImages() const { return getImagesMod(); }
		inline VkImage getSwapchainImage(uint32_t index) const { assert(index < getImageCount()); return getSwapchainImages()[index]; }
		inline VkImage getCurrentSwapchainImage() const { return getSwapchainImages()[imageIndex]; }

		inline const VkImageView* getSwapchainImageViews() const { return getImageViewsMod(); }
		inline VkImageView getSwapchainImageView(uint32_t index) const { assert(index < attachmentCount); return getSwapchainImageViews()[index]; }
		inline VkImageView getCurrentSwapchainImageView() const { return getSwapchainImageViews()[imageIndex]; }

		inline const VkFramebuffer* getFramebuffers() const { return getFramebuffersMod(); }
		inline VkFramebuffer getFramebuffer(uint32_t index) const { assert(index < getImageCount()); return getFramebuffers()[index]; }
		inline VkFramebuffer getCurrentFramebuffer() const { return getFramebuffer(getImageIndex()); }

		inline uint32_t getAttachmentCount() const { return attachmentCount; }
		inline const VkFormat* getAttachmentFormats() const { return getAttachmentFormatsMod(); }
		inline VkFormat getAttachmentFormat(uint32_t index) const { assert(index < getAttachmentCount()); return getAttachmentFormats()[index]; }

		inline const VkSampleCountFlagBits* getAttachmentSampleCounts() const { return getAttachmentSampleCountsMod(); }
		inline VkSampleCountFlagBits getAttachmentSampleCount(uint32_t index) const { assert(index < getAttachmentCount()); return getAttachmentSampleCounts()[index]; }
		inline const VkImageUsageFlags* getAttachmentUsages() const { return getAttachmentUsagesMod(); }
		inline VkImageUsageFlags getAttachmentUsage(uint32_t index) const { assert(index < getAttachmentCount()); return getAttachmentUsages()[index]; }

		inline const VkImage* getAttachmentImages() const { return getAttachmentImagesMod(); }
		inline VkImage getAttachmentImage(uint32_t index) const { assert(index < getAttachmentCount()); return getAttachmentImages()[index]; }
		inline const VkImageView* getAttachmentImageViews() const { return getAttachmentImageViewsMod(); }
		inline VkImageView getAttachmentImageView(uint32_t index) const { assert(index < getAttachmentCount()); return getAttachmentImageViews()[index]; }

		inline const VkClearValue* getClearValues() const { return getClearValuesMod(); }
		inline VkClearValue getClearValue(uint32_t index) { assert(index < getAttachmentCount()); return getClearValues()[index]; }
        inline uint32_t getClearValueCount() const { return getAttachmentCount(); }

		inline VkPresentModeKHR getPresentMode() const { return presentMode; }
		inline VkSurfaceFormatKHR getSurfaceFormat() const { return surfaceFormat; }
		inline VkFormat getImageFormat() const { return surfaceFormat.format; }
		inline VkColorSpaceKHR getColorSpace() const { return surfaceFormat.colorSpace; }

        //====================================================================
        //========================Window Modifiers============================
        //====================================================================
        bool isFullscreen() const;
        bool isMinimized() const;
        void enableVsync();
        void disableVsync();
        void setFullscreen();
        void setWindowed(uint32_t width, uint32_t height);
        void resize(uint32_t width, uint32_t height);
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
        inline Window() {}

        void updateSwapchain();
        void createFramebuffers();
		void destroyFramebuffers();
        void updateFramebuffers();
		void allocateAttachmentData(const VkAttachmentDescription* pAttachments, const VkClearValue* pClearValues, uint32_t attachmentCount, const VkSubpassDescription* pSubpasses, uint32_t subpassCount);
        void freeAttachmentData();
        void createSwapchain();
        void destroySwapchain();

        inline size_t getImageOffset() const { return 0; }
        inline size_t getImageViewOffset() const { return sizeof(VkImage) * imageCount; }
        inline size_t getFramebufferOffset() const { return getImageViewOffset() + sizeof(VkImageView) * imageCount; }
        inline size_t getAttachmentImagesOffset() const { return getFramebufferOffset() + sizeof(VkFramebuffer) * imageCount; }
        inline size_t getAttachmentImageViewsOffset() const { return getAttachmentImagesOffset() + sizeof(VkImage) * attachmentCount; }
        inline size_t getAttachmentMemoryOffset() const { return getAttachmentImageViewsOffset() + sizeof(VkImageView) * attachmentCount; }
        inline size_t getAttachmentFormatsOffset() const { return getAttachmentMemoryOffset() + sizeof(VkDeviceMemory) * (attachmentCount % 1); }
        inline size_t getAttachmentUsagesOffset() const { return getAttachmentFormatsOffset() + sizeof(VkFormat) * attachmentCount; }
        inline size_t getAttachmentSampleOffset() const { return getAttachmentUsagesOffset() + sizeof(VkImageUsageFlags) * attachmentCount; }
        inline size_t getClearValuesOffset() const { return getAttachmentSampleOffset() + sizeof(VkSampleCountFlagBits) * attachmentCount; }

        inline VkImage* getImagesMod() const { assert(attachmentData != nullptr); return (VkImage*)(attachmentData + getImageOffset()); }
        inline VkImageView* getImageViewsMod() const { assert(attachmentData != nullptr); return (VkImageView*)(attachmentData + getImageViewOffset()); }
        inline VkFramebuffer* getFramebuffersMod() const { assert(attachmentData != nullptr); return (VkFramebuffer*)(attachmentData + getFramebufferOffset()); }
        inline VkImage* getAttachmentImagesMod() const { assert(attachmentData != nullptr); return (VkImage*)(attachmentData + getAttachmentImagesOffset()); }
        inline VkImageView* getAttachmentImageViewsMod() const { assert(attachmentData != nullptr); return (VkImageView*)(attachmentData + getAttachmentImageViewsOffset()); }
        inline VkDeviceMemory& getAttachmentMemory() const { assert(attachmentData != nullptr && attachmentCount != 0); return *(VkDeviceMemory*)(attachmentData + getAttachmentMemoryOffset()); }
        inline void setAttachmentMemory(VkDeviceMemory memory) const { getAttachmentMemory() = memory; }
        inline VkFormat* getAttachmentFormatsMod() const { assert(attachmentData != nullptr); return (VkFormat*)(attachmentData + getAttachmentFormatsOffset()); }
        inline VkImageUsageFlags* getAttachmentUsagesMod() const { assert(attachmentData != nullptr); return (VkImageUsageFlags*)(attachmentData + getAttachmentUsagesOffset()); }
        inline VkSampleCountFlagBits* getAttachmentSampleCountsMod() const { assert(attachmentData != nullptr); return (VkSampleCountFlagBits*)(attachmentData + getAttachmentSampleOffset()); }
        inline VkClearValue* getClearValuesMod() const { assert(attachmentData != nullptr); return (VkClearValue*)(attachmentData + getClearValuesOffset()); }
        

        friend Device;
        //VkExtent2D extent = { 0, 0 };
        uint32_t width = 0;
        uint32_t height = 0;
        //void* window = nullptr;
        WindowHandle windowHandle;
        VkSurfaceKHR surface = VK_NULL_HANDLE;
        VkSwapchainKHR swapchain = VK_NULL_HANDLE;
    	VkQueue presentQueue = VK_NULL_HANDLE;
		VkRenderPass renderPass = VK_NULL_HANDLE;
		VkSurfaceFormatKHR surfaceFormat = DEFAULT_SURFACE_FORMAT;
        VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;
        WindowCreateFlags windowFlags = FLAG_NONE;
        VkSampleCountFlagBits sampleCount = VK_SAMPLE_COUNT_1_BIT;
		uint32_t imageCount = 0;
		uint32_t imageIndex = 0;
		uint32_t attachmentCount = 0;
        char* attachmentData = nullptr;

        //Display display;
        static Window s_MainWindow;
        inline static void* s_NativeFocused = nullptr;
        static WindowSettings s_WindowSettings;
    };
}