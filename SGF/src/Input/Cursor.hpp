#pragma once


namespace SGF {
    class Cursor {
    public:
        static const Cursor STANDARD;
        Cursor(const char* filename);
        ~Cursor();
        inline void* GetHandle() const { return handle; }
    private:
        inline Cursor() : handle(nullptr) {}
        void* handle;
    };
}