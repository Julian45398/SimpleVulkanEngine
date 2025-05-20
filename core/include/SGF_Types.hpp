#pragma once

#include <stdint.h>

namespace SGF {
    typedef uint32_t Flags;
    inline constexpr Flags FLAG_NONE = 0;
    class Window;
    class Device;
    class Swapchain;
    class RenderPass;
    class DeviceMemory;
    class Buffer;
    class Image;
    class ImageView;

    // Events: 
    class WindowResizeEvent;
    class WindowCloseEvent;
    class WindowMinimizeEvent;
    class MouseMovedEvent;
    class MouseButtonEvent;
    class KeyPressedEvent;
    class KeyTypedEvent;
    class DeviceDestroyEvent;
    // Layer:
    class Layer;
    class LayerStack;
}

