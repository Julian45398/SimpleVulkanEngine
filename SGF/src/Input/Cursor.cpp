#include "Cursor.hpp"

#include "Filesystem/File.hpp"
#include <GLFW/glfw3.h>

namespace SGF {
    const Cursor Cursor::STANDARD;
    Cursor::Cursor(const char* textureFile) {
        GLFWimage image;
        auto data = LoadTextureFile(textureFile, (uint32_t*)&image.width, (uint32_t*)&image.height);
        image.pixels = data.data();
        handle = glfwCreateCursor(&image, 0, 0);
    }
    Cursor::~Cursor() { glfwDestroyCursor((GLFWcursor*)handle); }
}