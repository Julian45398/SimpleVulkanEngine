#pragma once

#include "SGF_Core.hpp"
#include "Input/Keycodes.hpp"
#include "Input/Mousecodes.hpp"
#include "Input/Cursor.hpp"

namespace SGF {
    struct FileFilter {
		inline FileFilter(const char* description, const char* filter) : filterDescription(description), filters(filter) {}
		inline FileFilter() : filterDescription(nullptr), filters(nullptr) {}
		const char* filterDescription;
		const char* filters;
	};

    class WindowHandle {
    public:
        void Open(const char* title, uint32_t width, uint32_t height, WindowCreateFlags windowFlags);
        void Close();
        inline WindowHandle(const char* title, uint32_t width, uint32_t height, WindowCreateFlags windowFlags) 
        { WindowHandle::Open(title, width, height, windowFlags); }
        inline WindowHandle() : nativeHandle(nullptr) {};
        inline WindowHandle(void* handle) : nativeHandle(handle) {};
        inline void SetHandle(void* handle) { nativeHandle = handle; }

        bool ShouldClose() const;

        uint32_t GetWidth() const;
        uint32_t GetHeight() const;
        glm::uvec2 GetSize() const;
        glm::uvec2 GetFramebufferSize() const;

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
        void* nativeHandle;
    };
}