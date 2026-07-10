#pragma once

#include <SGF/Core/Math/Math.hpp>
#include <SGF/Core/Flags.hpp>
#include <SGF/Core/Macros.hpp>
#include <SGF/Core/Layers/LayerStack.hpp>
#include <string>

#include "InputMappings.hpp"
#include "Cursor.hpp"

namespace SGF {
    struct FileFilter {
		inline FileFilter(const char* description, const char* filter) : filterDescription(description), filters(filter) {}
		inline FileFilter() : filterDescription(nullptr), filters(nullptr) {}
		const char* filterDescription;
		const char* filters;
	};

    enum class WindowOptions {
        NONE = 0,
        FULLSCREEN = SGF_BIT(0),
        RESIZABLE = SGF_BIT(1),
        BORDERLESS = SGF_BIT(2),
        VSYNC = SGF_BIT(8)
	};

    class WindowHandle {
    public:
        void Open(const char* title, uint32_t width, uint32_t height, Flags<WindowOptions> windowFlags, const LayerStack* layerStack = nullptr);
        void Close();
        inline WindowHandle(const char* title, uint32_t width, uint32_t height, Flags<WindowOptions> windowFlags, const LayerStack* layerStack = nullptr) 
        { WindowHandle::Open(title, width, height, windowFlags); }
        inline WindowHandle() : m_BackendHandle(nullptr) {};
        inline WindowHandle(void* handle) : m_BackendHandle(handle) {};
        inline void SetHandle(void* handle) { m_BackendHandle = handle; }

        bool ShouldClose() const;

        uint32_t GetWidth() const;
        uint32_t GetHeight() const;
        glm::uvec2 GetSize() const;
        glm::uvec2 GetFramebufferSize() const;

        bool IsKeyPressed(Key key) const;
        bool IsMouseButtonPressed(MouseButton button) const;

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
        inline bool IsOpen() const { return m_BackendHandle != nullptr; }

        inline void* GetNativeHandle() const { return m_BackendHandle; }
        void SetLayerEventCallbacks(const LayerStack* layerStack) const;

        void SetTitle(const char* title) const;
        const char* GetTitle() const;

        void SetFullscreen() const;
        void SetWindowed(uint32_t width, uint32_t height) const;
        void SetFocused() const;
        void Resize(uint32_t width, uint32_t height) const;
        void Minimize() const;
        void Restore() const;

		std::string OpenFileDialog(const FileFilter* pFilters, uint32_t filterCount) const;
		inline std::string OpenFileDialog(const FileFilter& filter) const { return OpenFileDialog(&filter, 1); }
		inline std::string OpenFileDialog(const char* filterDescription, const char* filter) const { return OpenFileDialog(FileFilter(filterDescription, filter)); }
		inline std::string OpenFileDialog(const std::vector<FileFilter>& filters) const { return OpenFileDialog(filters.data(), filters.size()); }
        template<uint32_t COUNT>
		inline std::string OpenFileDialog(const FileFilter(&filters)[COUNT]) const { return OpenFileDialog(filters, COUNT); }

		std::string SaveFileDialog(const FileFilter* pFilters, uint32_t filterCount) const;
		inline std::string SaveFileDialog(const FileFilter& filter) const { return SaveFileDialog(&filter, 1); }
		inline std::string SaveFileDialog(const char* filterDescription, const char* filter) const { return SaveFileDialog(FileFilter(filterDescription, filter)); }
		inline std::string SaveFileDialog(const std::vector<FileFilter>& filters) const { return SaveFileDialog(filters.data(), filters.size()); }
        template<uint32_t COUNT>
		inline std::string SaveFileDialog(const FileFilter(&filters)[COUNT]) const { return SaveFileDialog(filters, COUNT); }
    private:
        void* m_BackendHandle;
    };
}