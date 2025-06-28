#pragma once

#include "SGF_Core.hpp"
#include "Input/Keycodes.hpp"
#include "Input/Mousecodes.hpp"
#include <string.h>

namespace SGF {
    
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
    class Cursor {
    public:
        static const Cursor STANDARD;
        Cursor(const char* filename);
        ~Cursor();
    private:
        inline Cursor() : handle(nullptr) {}
        friend WindowHandle;
        void* handle;
    };
    class WindowHandle {
    public:
        void Open(const char* title, uint32_t width, uint32_t height, WindowCreateFlags windowFlags);
        void Close();
        inline WindowHandle(const char* title, uint32_t width, uint32_t height, WindowCreateFlags windowFlags) 
        { Open(title, width, height, windowFlags); }
        inline WindowHandle() : nativeHandle(nullptr) {};
        inline WindowHandle(void* handle) : nativeHandle(handle) {};
        inline void SetHandle(void* handle) { nativeHandle = handle; }

        bool ShouldClose() const;
        uint32_t GetWidth() const;
        uint32_t GetHeight() const;
        VkExtent2D GetSize() const;
        bool IsKeyPressed(Keycode key) const;
        bool IsMouseButtonPressed(Mousecode button) const;

        glm::dvec2 GetCursorPos() const;
        void SetCursorPos(double xpos, double ypos) const;
        void SetCursorPos(const glm::dvec2& pos) const;
        void CaptureCursor() const;
        void HideCursor() const;
        void RestrictCursor() const;
        void FreeCursor() const;
        void SetCursor(const Cursor& cursor) const;

        bool IsFullscreen() const;
        bool IsMinimized() const;
        bool IsFocused() const;
        inline bool IsOpen() const { return nativeHandle != nullptr; }

        inline void* GetHandle() const { return nativeHandle; }
        void SetUserPointer(void* pUser) const;

        void SetTitle(const char* title) const;
        const char* GetTitle() const;

        void SetFullscreen() const;
        void SetWindowed(uint32_t width, uint32_t height) const;
        void Resize(uint32_t width, uint32_t height) const;
        void Minimize() const;

		std::string OpenFileDialog(const FileFilter* pFilters, uint32_t filterCount) const;
		inline std::string OpenFileDialog(const FileFilter& filter) const { return OpenFileDialog(&filter, 1); }
		inline std::string OpenFileDialog(const char* filterDescription, const char* filter) const { return OpenFileDialog((FileFilter){filterDescription, filter}); }
		inline std::string OpenFileDialog(const std::vector<FileFilter>& filters) const { return OpenFileDialog(filters.data(), filters.size()); }
        template<uint32_t COUNT>
		inline std::string OpenFileDialog(const FileFilter(&filters)[COUNT]) const { return OpenFileDialog(filters, COUNT); }

		std::string SaveFileDialog(const FileFilter* pFilters, uint32_t filterCount) const;
		inline std::string SaveFileDialog(const FileFilter& filter) const { return SaveFileDialog(&filter, 1); }
		inline std::string SaveFileDialog(const char* filterDescription, const char* filter) const { return SaveFileDialog((FileFilter){filterDescription, filter}); }
		inline std::string SaveFileDialog(const std::vector<FileFilter>& filters) const { return SaveFileDialog(filters.data(), filters.size()); }
        template<uint32_t COUNT>
		inline std::string SaveFileDialog(const FileFilter(&filters)[COUNT]) const { return SaveFileDialog(filters, COUNT); }
    private:
        void* nativeHandle;
    };
    class Input {
    public:
        static void PollEvents();
        static void WaitEvents();
        static bool HasFocus();
        static glm::dvec2 GetCursorPos();
        static void SetCursorPos(double xpos, double ypos);
        static void SetCursorPos(const glm::dvec2& pos);
        static void CaptureCursor();
        static void HideCursor();
        static void RestrictCursor();
        static void FreeCursor();
        static void SetCursor(const Cursor& cursor);

        static bool IsMouseButtonPressed(Mousecode button);
        static bool IsKeyPressed(Keycode key);
        inline static WindowHandle& GetFocusedWindow() { return s_FocusedWindow; }
    private:
        friend WindowHandle;
        inline static WindowHandle s_FocusedWindow;
    };

    class Window {
    public:
		inline static const VkSurfaceFormatKHR DEFAULT_SURFACE_FORMAT = { VK_FORMAT_B8G8R8A8_UNORM, VK_COLORSPACE_SRGB_NONLINEAR_KHR };
        static VkAttachmentDescription CreateSwapchainAttachment(VkAttachmentLoadOp loadOp);
        inline static WindowHandle GetFocusedHandle() { return Input::GetFocusedWindow(); }
        //inline static Window* GetFocused() { return FocusedWindow; }
        static VkExtent2D GetMonitorSize(uint32_t index);
        static VkExtent2D GetMaxMonitorSize();
        static uint32_t GetMonitorCount();
        inline static Window& Get() { return s_MainWindow; }
        static void CreateMain() { Get().Open(s_WindowSettings.title, s_WindowSettings.width, s_WindowSettings.height, s_WindowSettings.createFlags); }
        inline static void SetCreateFlags(WindowCreateFlags flags) { s_WindowSettings.createFlags = flags; }
        inline static void SetSize(uint32_t width, uint32_t height) { s_WindowSettings.width = width, s_WindowSettings.height = height; }
        inline static void SetTitle(const char* title) { assert(strlen(title) < ARRAY_SIZE(s_WindowSettings.title)); strncpy(s_WindowSettings.title, title, ARRAY_SIZE(s_WindowSettings.title)); }
        inline static void EnableClear(float r, float g, float b, float a);
        inline static void CloseMain() { Get().Close(); }

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
        { Open(name, width, height, flags, multisampleCount); }

        inline ~Window() { Close(); }

        Window(Window&& other) = delete;
        Window(const Window& other) = delete;
        Window(Window& other) = delete;
        
        void Open(const char* name, uint32_t width, uint32_t height, WindowCreateFlags flags = FLAG_NONE, VkSampleCountFlagBits multisampleCount = VK_SAMPLE_COUNT_1_BIT);
        void Close();

        void SetRenderPass(const VkAttachmentDescription* pAttachments, const VkClearValue* pClearValues, uint32_t attCount, const VkSubpassDescription* pSubpasses, uint32_t subpassCount, const VkSubpassDependency* pDependencies, uint32_t dependencyCount);

        template<size_t ATTACHMENT_COUNT, size_t SUBPASS_COUNT, size_t DEPENDENCY_COUNT>
        inline void SetRenderPass(const VkAttachmentDescription(&attachments)[ATTACHMENT_COUNT], const VkClearValue(&clearValues)[ATTACHMENT_COUNT], const VkSubpassDescription(&subpasses)[SUBPASS_COUNT], const VkSubpassDependency(&dependencies)[DEPENDENCY_COUNT])
        { SetRenderPass(attachments, clearValues, ATTACHMENT_COUNT, subpasses, SUBPASS_COUNT, dependencies, DEPENDENCY_COUNT); }
        
        inline void SetRenderPass(const std::vector<VkAttachmentDescription>& attachments, const std::vector<VkSubpassDescription>& subpasses, const std::vector<VkSubpassDependency>& dependencies, const std::vector<VkClearValue>& clearValues)
        { assert(attachments.size() == clearValues.size()); SetRenderPass(attachments.data(), clearValues.data(), (uint32_t)attachments.size(), subpasses.data(), (uint32_t)subpasses.size(), dependencies.data(), (uint32_t)dependencies.size()); }

        inline void ResizeFramebuffers(uint32_t w, uint32_t h) { width = w; height = h; UpdateFramebuffers(); }

		inline void RequestSurfaceFormat(VkSurfaceFormatKHR requested) { surfaceFormat = requested; }
		inline void RequestColorSpace(VkColorSpaceKHR requested) { surfaceFormat.colorSpace = requested; }
		inline void RequestImageFormat(VkFormat requested) { surfaceFormat.format = requested; }
		inline void RequestPresentMode(VkPresentModeKHR requested) { presentMode = requested; }

        void NextFrame(VkSemaphore imageAvailableSignal, VkFence fence = VK_NULL_HANDLE);
		void PresentFrame(const VkSemaphore* pWaitSemaphores, uint32_t waitCount);
        template<uint32_t WAIT_COUNT>
        inline void PresentFrame(const VkSemaphore(&waitSemaphores)[WAIT_COUNT]) { PresentFrame(waitSemaphores, WAIT_COUNT); }
        inline void PresentFrame(const std::vector<VkSemaphore>& waitSemaphores) { PresentFrame(waitSemaphores.data(), (uint32_t)waitSemaphores.size()); }
		inline void PresentFrame(VkSemaphore waitSemaphore) { PresentFrame(&waitSemaphore, 1); }
        //====================================================================
        //========================Window Information==========================
        //====================================================================
        bool IsOpen() const { return windowHandle.IsOpen(); }
        bool HasRenderTarget() const { return renderPass != nullptr; }
        bool ShouldClose() const;

        inline uint32_t GetWidth() const { return width; }
        inline uint32_t GetHeight() const { return height; }
        inline VkExtent2D GetFramebufferSize() const { return {width, height}; }
        inline VkSurfaceKHR GetSurface() const { return surface; }
        inline VkRenderPass GetRenderPass() const { return renderPass; }
		inline VkSwapchainKHR GetSwapchain() const { return swapchain; }

        inline uint32_t GetImageIndex() const { return imageIndex; }
        inline uint32_t GetImageCount() const { return imageCount; }
        inline operator VkSurfaceKHR() const { return surface; }
        inline const WindowHandle& GetNativeWindow() const { return windowHandle; }
        const char* GetName() const;


		inline const VkImage* GetSwapchainImages() const { return GetImagesMod(); }
		inline VkImage GetSwapchainImage(uint32_t index) const { assert(index < GetImageCount()); return GetSwapchainImages()[index]; }
		inline VkImage GetCurrentSwapchainImage() const { return GetSwapchainImages()[imageIndex]; }

		inline const VkImageView* GetSwapchainImageViews() const { return GetImageViewsMod(); }
		inline VkImageView GetSwapchainImageView(uint32_t index) const { assert(index < attachmentCount); return GetSwapchainImageViews()[index]; }
		inline VkImageView GetCurrentSwapchainImageView() const { return GetSwapchainImageViews()[imageIndex]; }

		inline const VkFramebuffer* GetFramebuffers() const { return GetFramebuffersMod(); }
		inline VkFramebuffer GetFramebuffer(uint32_t index) const { assert(index < GetImageCount()); return GetFramebuffers()[index]; }
		inline VkFramebuffer GetCurrentFramebuffer() const { return GetFramebuffer(GetImageIndex()); }

		inline uint32_t GetAttachmentCount() const { return attachmentCount; }
		inline const VkFormat* GetAttachmentFormats() const { return GetAttachmentFormatsMod(); }
		inline VkFormat GetAttachmentFormat(uint32_t index) const { assert(index < GetAttachmentCount()); return GetAttachmentFormats()[index]; }

		inline const VkSampleCountFlagBits* GetAttachmentSampleCounts() const { return GetAttachmentSampleCountsMod(); }
		inline VkSampleCountFlagBits GetAttachmentSampleCount(uint32_t index) const { assert(index < GetAttachmentCount()); return GetAttachmentSampleCounts()[index]; }
		inline const VkImageUsageFlags* GetAttachmentUsages() const { return GetAttachmentUsagesMod(); }
		inline VkImageUsageFlags GetAttachmentUsage(uint32_t index) const { assert(index < GetAttachmentCount()); return GetAttachmentUsages()[index]; }

		inline const VkImage* GetAttachmentImages() const { return GetAttachmentImagesMod(); }
		inline VkImage GetAttachmentImage(uint32_t index) const { assert(index < GetAttachmentCount()); return GetAttachmentImages()[index]; }
		inline const VkImageView* GetAttachmentImageViews() const { return GetAttachmentImageViewsMod(); }
		inline VkImageView GetAttachmentImageView(uint32_t index) const { assert(index < GetAttachmentCount()); return GetAttachmentImageViews()[index]; }

		inline const VkClearValue* GetClearValues() const { return GetClearValuesMod(); }
		inline VkClearValue GetClearValue(uint32_t index) { assert(index < GetAttachmentCount()); return GetClearValues()[index]; }
        inline uint32_t GetClearValueCount() const { return GetAttachmentCount(); }

		inline VkPresentModeKHR GetPresentMode() const { return presentMode; }
		inline VkSurfaceFormatKHR GetSurfaceFormat() const { return surfaceFormat; }
		inline VkFormat GetImageFormat() const { return surfaceFormat.format; }
		inline VkColorSpaceKHR GetColorSpace() const { return surfaceFormat.colorSpace; }

        //====================================================================
        //========================Window Modifiers============================
        //====================================================================
        bool IsFullscreen() const;
        bool IsMinimized() const;
        void EnableVsync();
        void DisableVsync();
        void SetFullscreen();
        void SetWindowed(uint32_t width, uint32_t height);
        void Resize(uint32_t width, uint32_t height);
        void Minimize();

        //====================================================================
        //============================Window Input============================
        //====================================================================
        bool IsKeyPressed(Keycode key) const;
        bool IsMousePressed(Mousecode button) const;
        glm::dvec2 GetCursorPos() const;

    private:
        inline Window() {}

        void UpdateSwapchain();
        void CreateFramebuffers();
		void DestroyFramebuffers();
        void UpdateFramebuffers();
		void AllocateAttachmentData(const VkAttachmentDescription* pAttachments, const VkClearValue* pClearValues, uint32_t attachmentCount, const VkSubpassDescription* pSubpasses, uint32_t subpassCount);
        void FreeAttachmentData();
        void CreateSwapchain();
        void DestroySwapchain();

        inline size_t GetImageOffset() const { return 0; }
        inline size_t GetImageViewOffset() const { return sizeof(VkImage) * imageCount; }
        inline size_t GetFramebufferOffset() const { return GetImageViewOffset() + sizeof(VkImageView) * imageCount; }
        inline size_t GetAttachmentImagesOffset() const { return GetFramebufferOffset() + sizeof(VkFramebuffer) * imageCount; }
        inline size_t GetAttachmentImageViewsOffset() const { return GetAttachmentImagesOffset() + sizeof(VkImage) * attachmentCount; }
        inline size_t GetAttachmentMemoryOffset() const { return GetAttachmentImageViewsOffset() + sizeof(VkImageView) * attachmentCount; }
        inline size_t GetAttachmentFormatsOffset() const { return GetAttachmentMemoryOffset() + sizeof(VkDeviceMemory) * (attachmentCount % 1); }
        inline size_t GetAttachmentUsagesOffset() const { return GetAttachmentFormatsOffset() + sizeof(VkFormat) * attachmentCount; }
        inline size_t GetAttachmentSampleOffset() const { return GetAttachmentUsagesOffset() + sizeof(VkImageUsageFlags) * attachmentCount; }
        inline size_t GetClearValuesOffset() const { return GetAttachmentSampleOffset() + sizeof(VkSampleCountFlagBits) * attachmentCount; }

        inline VkImage* GetImagesMod() const { assert(attachmentData != nullptr); return (VkImage*)(attachmentData + GetImageOffset()); }
        inline VkImageView* GetImageViewsMod() const { assert(attachmentData != nullptr); return (VkImageView*)(attachmentData + GetImageViewOffset()); }
        inline VkFramebuffer* GetFramebuffersMod() const { assert(attachmentData != nullptr); return (VkFramebuffer*)(attachmentData + GetFramebufferOffset()); }
        inline VkImage* GetAttachmentImagesMod() const { assert(attachmentData != nullptr); return (VkImage*)(attachmentData + GetAttachmentImagesOffset()); }
        inline VkImageView* GetAttachmentImageViewsMod() const { assert(attachmentData != nullptr); return (VkImageView*)(attachmentData + GetAttachmentImageViewsOffset()); }
        inline VkDeviceMemory& GetAttachmentMemory() const { assert(attachmentData != nullptr && attachmentCount != 0); return *(VkDeviceMemory*)(attachmentData + GetAttachmentMemoryOffset()); }
        inline void SetAttachmentMemory(VkDeviceMemory memory) const { GetAttachmentMemory() = memory; }
        inline VkFormat* GetAttachmentFormatsMod() const { assert(attachmentData != nullptr); return (VkFormat*)(attachmentData + GetAttachmentFormatsOffset()); }
        inline VkImageUsageFlags* GetAttachmentUsagesMod() const { assert(attachmentData != nullptr); return (VkImageUsageFlags*)(attachmentData + GetAttachmentUsagesOffset()); }
        inline VkSampleCountFlagBits* GetAttachmentSampleCountsMod() const { assert(attachmentData != nullptr); return (VkSampleCountFlagBits*)(attachmentData + GetAttachmentSampleOffset()); }
        inline VkClearValue* GetClearValuesMod() const { assert(attachmentData != nullptr); return (VkClearValue*)(attachmentData + GetClearValuesOffset()); }
        

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
        inline static WindowHandle* s_NativeFocused = nullptr;
        static WindowSettings s_WindowSettings;
    };
}