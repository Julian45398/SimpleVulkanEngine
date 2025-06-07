#pragma once

#include <stdint.h>
#include "SGF_Macros.hpp"
#include <vector>

namespace SGF {
    typedef uint32_t Flags;
    inline constexpr Flags FLAG_NONE = 0;
    class Window;
    class Device;
    class Swapchain;
    class Display;
    class RenderPass;
    class DeviceMemory;
    class Buffer;
    class Image;
    class ImageView;
    class CommandList;

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

    enum WindowCreateFlagBits {
        WINDOW_FLAG_FULLSCREEN = BIT(0),
        WINDOW_FLAG_RESIZABLE = BIT(1),
        WINDOW_FLAG_MINIMIZED = BIT(2),
        WINDOW_FLAG_MAXIMIZED = BIT(3)
    };
    typedef Flags WindowCreateFlags;

    enum DeviceFeatureFlagBits : uint64_t {
        DEVICE_FEATURE_ROBUST_BUFFER_ACCESS = BIT(0),
        DEVICE_FEATURE_FULL_DRAW_INDEX_UINT32 = BIT(1),
        DEVICE_FEATURE_IMAGE_CUBE_ARRAY = BIT(2),
        DEVICE_FEATURE_INDEPENDENT_BLEND = BIT(3),
        DEVICE_FEATURE_GEOMETRY_SHADER = BIT(4),
        DEVICE_FEATURE_TESSELLATION_SHADER = BIT(5),
        DEVICE_FEATURE_SAMPLE_RATE_SHADING = BIT(6),
        DEVICE_FEATURE_DUAL_SRC_BLEND = BIT(7),
        DEVICE_FEATURE_LOGIC_OP = BIT(8),
        DEVICE_FEATURE_MULTI_DRAW_INDIRECT = BIT(9),
        DEVICE_FEATURE_DRAW_INDIRECT_FIRST_INSTANCE = BIT(10),
        DEVICE_FEATURE_DEPTH_CLAMP = BIT(11),
        DEVICE_FEATURE_DEPTH_BIAS_CLAMP = BIT(12),
        DEVICE_FEATURE_FILL_MODE_NON_SOLID = BIT(13),
        DEVICE_FEATURE_DEPTH_BOUNDS = BIT(14),
        DEVICE_FEATURE_WIDE_LINES = BIT(15),
        DEVICE_FEATURE_LARGE_POINTS = BIT(16),
        DEVICE_FEATURE_ALPHA_TO_ONE = BIT(17),
        DEVICE_FEATURE_MULTI_VIEWPORT = BIT(18),
        DEVICE_FEATURE_SAMPLER_ANISOTROPY = BIT(19),
        DEVICE_FEATURE_TEXTURE_COMPRESSION_ETC2 = BIT(20),
        DEVICE_FEATURE_TEXTURE_COMPRESSION_ASTC_LDR = BIT(21),
        DEVICE_FEATURE_TEXTURE_COMPRESSION_BC = BIT(22),
        DEVICE_FEATURE_OCCLUSION_QUERY_PRECISE = BIT(23),
        DEVICE_FEATURE_PIPELINE_STATISTICS_QUERY = BIT(24),
        DEVICE_FEATURE_VERTEX_PIPELINE_STORES_AND_ATOMICS = BIT(25),
        DEVICE_FEATURE_FRAGMENT_STORES_AND_ATOMICS = BIT(26),
        DEVICE_FEATURE_SHADER_TESSELLATION_AND_GEOMETRY_POINT_SIZE = BIT(27),
        DEVICE_FEATURE_SHADER_IMAGE_GATHER_EXTENDED = BIT(28),
        DEVICE_FEATURE_SHADER_STORAGE_IMAGE_EXTENDED_FORMATS = BIT(29),
        DEVICE_FEATURE_SHADER_STORAGE_IMAGE_MULTISAMPLE = BIT(30),
        DEVICE_FEATURE_SHADER_STORAGE_IMAGE_READ_WITHOUT_FORMAT = BIT(31),
        DEVICE_FEATURE_SHADER_STORAGE_IMAGE_WRITE_WITHOUT_FORMAT = BIT(32),
        DEVICE_FEATURE_SHADER_UNIFORM_BUFFER_ARRAY_DYNAMIC_INDEXING = BIT(33),
        DEVICE_FEATURE_SHADER_SAMPLED_IMAGE_ARRAY_DYNAMIC_INDEXING = BIT(34),
        DEVICE_FEATURE_SHADER_STORAGE_BUFFER_ARRAY_DYNAMIC_INDEXING = BIT(35),
        DEVICE_FEATURE_SHADER_STORAGE_IMAGE_ARRAY_DYNAMIC_INDEXING = BIT(36),
        DEVICE_FEATURE_SHADER_CLIP_DISTANCE = BIT(37),
        DEVICE_FEATURE_SHADER_CULL_DISTANCE = BIT(38),
        DEVICE_FEATURE_SHADER_FLOAT64 = BIT(39),
        DEVICE_FEATURE_SHADER_INT64 = BIT(40),
        DEVICE_FEATURE_SHADER_INT16 = BIT(41),
        DEVICE_FEATURE_SHADER_RESOURCE_RESIDENCY = BIT(42),
        DEVICE_FEATURE_SHADER_RESOURCE_MIN_LOD = BIT(43),
        DEVICE_FEATURE_SPARSE_BINDING = BIT(44),
        DEVICE_FEATURE_SPARSE_RESIDENCY_BUFFER = BIT(45),
        DEVICE_FEATURE_SPARSE_RESIDENCY_IMAGE2D = BIT(46),
        DEVICE_FEATURE_SPARSE_RESIDENCY_IMAGE3D = BIT(47),
        DEVICE_FEATURE_SPARSE_RESIDENCY_2_SAMPLES = BIT(48),
        DEVICE_FEATURE_SPARSE_RESIDENCY_4_SAMPLES = BIT(49),
        DEVICE_FEATURE_SPARSE_RESIDENCY_8_SAMPLES = BIT(50),
        DEVICE_FEATURE_SPARSE_RESIDENCY_16_SAMPLES = BIT(51),
        DEVICE_FEATURE_SPARSE_RESIDENCY_ALIASED = BIT(52),
        DEVICE_FEATURE_VARIABLE_MULTISAMPLE_RATE = BIT(53),
        DEVICE_FEATURE_INHERITED_QUERIES = BIT(54),
        DEVICE_FEATURE_MAX_ENUM = BIT(55)
    };
    typedef uint64_t DeviceFeatureFlags;
    static_assert(sizeof(DeviceFeatureFlagBits) == 8, "Feature flag bits must be 64 bits long!");

    enum QueueFamilyFlagBits {
        QUEUE_FAMILY_GRAPHICS = BIT(0),
        QUEUE_FAMILY_COMPUTE = BIT(1),
        QUEUE_FAMILY_TRANSFER = BIT(2),
        QUEUE_FAMILY_PRESENT = BIT(3)
    };
    typedef Flags QueueFamilyFlags;

    struct WindowSettings {
        char title[256];
        WindowCreateFlags createFlags;
        uint32_t width;
        uint32_t height;
    };
    struct DeviceRequirements {
        std::vector<const char*> extensions;
        DeviceFeatureFlags requiredFeatures;
        DeviceFeatureFlags optionalFeatures;
        uint32_t graphicsQueueCount;
        uint32_t computeQueueCount;
        uint32_t transferQueueCount;
    };
}

