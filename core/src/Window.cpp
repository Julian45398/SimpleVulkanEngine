#include "SGF_Core.hpp"
#include "Window.hpp"
#include "Device.hpp"
#include <assert.h>

namespace SGF {
    extern VkInstance VulkanInstance;
    extern VkAllocationCallbacks* VulkanAllocator;
}

#pragma region SWAPCHAIN_HELPER_FUNCTIONS

constexpr VkFormat POSSIBLE_DEPTH_FORMATS[] = { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT };

#define ARRAY_SIZE(X) (sizeof(X)/sizeof(X[0]))

VkSurfaceFormatKHR pickSurfaceFormat(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, VkSurfaceFormatKHR surfaceFormat) {
    uint32_t format_count;
    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &format_count, nullptr);
    std::vector<VkSurfaceFormatKHR> surface_formats(format_count);
    if (format_count != 0) {
        vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &format_count, surface_formats.data());
    }
    else {
        SGF::fatal("no surface format supported!");
    }
    for (const auto& available_format : surface_formats) {
        if (available_format.format == surfaceFormat.format && available_format.colorSpace == surfaceFormat.colorSpace)
        {
            return available_format;
        }
    }
    return surface_formats[0];
}


#pragma endregion SWAPCHAIN_HELPER_FUNCTIONS

SGF_Window::SGF_Window(const char* name, uint32_t width, uint32_t height, bool isFullscreen) {
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    if (isFullscreen) {
        window = glfwCreateWindow(width, height, name, glfwGetPrimaryMonitor(), nullptr);
    } else {
        window = glfwCreateWindow(width, height, name, nullptr, nullptr);
    }
    if (glfwCreateWindowSurface(SGF::VulkanInstance, window, SGF::VulkanAllocator, &surface) != VK_SUCCESS) {
        SGF::fatal("failed to create window: ", name);
    }
}
void SGF_Window::destroySwapchain() {
    assert(pDevice != nullptr);
    vkDestroyImage(pDevice->logical, depthImage, SGF::VulkanAllocator);
    vkDestroyImageView(pDevice->logical, depthImageView, SGF::VulkanAllocator);
    vkFreeMemory(pDevice->logical, depthMemory, SGF::VulkanAllocator);
    for(uint32_t i = 0; i < imageCount; ++i) {
        vkDestroyImageView(pDevice->logical, framebuffers[i].imageView, SGF::VulkanAllocator);
        vkDestroyFramebuffer(pDevice->logical, framebuffers[i].framebuffer, SGF::VulkanAllocator);
        vkDestroyFramebuffer(pDevice->logical, framebuffers[i].framebuffer, SGF::VulkanAllocator);
    }
    delete[] framebuffers;
    vkDestroySwapchainKHR(pDevice->logical, swapchain, SGF::VulkanAllocator);
}

void SGF_Window::enableVsync() {
    presentMode = VK_PRESENT_MODE_FIFO_KHR;
    createSwapchain();
}
void SGF_Window::disableVsync() {
    // find present modes: 
    uint32_t presentCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(pDevice->physical, surface, &presentCount, nullptr);
    if (presentCount != 0) {
        std::vector<VkPresentModeKHR> available(presentCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(pDevice->physical, surface, &presentCount, available.data());
        for (const auto& mode : available) {
            if (mode == VK_PRESENT_MODE_MAILBOX_KHR) {
                presentMode = mode;
                break;
            }
        }
        createSwapchain();
    }
}
void SGF_Window::createSwapchain() {
    //SGF_Device* pDevice = (SGF_Device*)glfwGetWindowUserPointer(window);
    assert(pDevice != nullptr);
    assert(pDevice->presentCount != 0);
    VkSwapchainCreateInfoKHR info{};
    info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    VkSurfaceCapabilitiesKHR capabilities;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(pDevice->physical, surface, &capabilities);
	imageCount = capabilities.minImageCount + 1;
	if (capabilities.maxImageCount > 0 && imageCount > capabilities.maxImageCount) {
        imageCount = capabilities.maxImageCount;
    }
    width = std::min(capabilities.maxImageExtent.width, std::max(width, capabilities.minImageExtent.width));
    height = std::min(capabilities.maxImageExtent.height, std::max(height, capabilities.minImageExtent.height));
	info.oldSwapchain = swapchain;
    info.surface = surface;
    info.minImageCount = imageCount;
    info.imageFormat = surfaceFormat.format;
    info.imageColorSpace = surfaceFormat.colorSpace;
    info.imageExtent = { width, height };
    info.imageArrayLayers = 1;
    info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    if (pDevice->presentFamily != pDevice->graphicsFamily) {
        uint32_t queueFamilyIndices[] = { pDevice->graphicsFamily, pDevice->presentFamily };
        info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        info.queueFamilyIndexCount = 2;
        info.pQueueFamilyIndices = queueFamilyIndices;
    }
    else {
        info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }
    info.preTransform = capabilities.currentTransform;
    info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    info.presentMode = presentMode;
    info.clipped = VK_TRUE;

	if (vkCreateSwapchainKHR(pDevice->logical, &info, SGF::VulkanAllocator, &swapchain) != VK_SUCCESS) {
        SGF::fatal("failed to create swapchain!");
    }
}
void SGF_Window::createResources() {
    SGF_Device* pDevice = (SGF_Device*)glfwGetWindowUserPointer(window);
    assert(pDevice != nullptr);
    // create resources:
	depthImage = pDevice->image2D(width, height, depthFormat, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);
    depthMemory = pDevice->allocate(depthImage);
    depthImageView = pDevice->imageView2D(depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);
    if (multiSampleState != VK_SAMPLE_COUNT_1_BIT) {
        SGF::warn("multisample not imlemented yet!");
    }

    vkGetSwapchainImagesKHR(pDevice->logical, swapchain, &imageCount, nullptr);
    std::vector<VkImage> images(imageCount);
    vkGetSwapchainImagesKHR(pDevice->logical, swapchain, &imageCount, images.data());
    framebuffers = new Framebuffer[images.size()];
    for (size_t i = 0; i < images.size(); ++i) {
        framebuffers[i].colorImage = images[i];
        framebuffers[i].imageView = pDevice->imageView2D(images[i], surfaceFormat.format, VK_IMAGE_ASPECT_COLOR_BIT);
        VkImageView attachments[] = { framebuffers[i].imageView, depthImageView };
        framebuffers[i].framebuffer = pDevice->framebuffer(renderPass, attachments, ARRAY_SIZE(attachments), width, height, 1);
    }
}

void SGF_Window::createRenderPass() {
    VkAttachmentDescription attachments[] = {
        {0, surfaceFormat.format, VK_SAMPLE_COUNT_1_BIT,
        VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE,
        VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
        VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR},
        {0, depthFormat, VK_SAMPLE_COUNT_1_BIT,
        VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_DONT_CARE,
        VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
        VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL}
    };
    VkAttachmentReference color_ref = {0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
    VkAttachmentReference depth_ref = {1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL};
    VkSubpassDescription subpasses[] = {
        {0, VK_PIPELINE_BIND_POINT_GRAPHICS, 0, nullptr, 1, &color_ref, nullptr, &depth_ref, 0, nullptr}
    };
    VkSubpassDependency dependencies[] = {
        {VK_SUBPASS_EXTERNAL, 0,
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
        0, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT}
    };
    renderPass = pDevice->renderPass(attachments, ARRAY_SIZE(attachments), subpasses, ARRAY_SIZE(subpasses), dependencies, ARRAY_SIZE(dependencies));
}

SGF_Window::~SGF_Window() {
    SGF::debug("Destroying window: ", glfwGetWindowTitle(window));
    SGF_Device* pDevice = (SGF_Device*)glfwGetWindowUserPointer(window);
    if (pDevice != nullptr) {
        destroySwapchain();
        vkDestroyRenderPass(pDevice->logical, renderPass, SGF::VulkanAllocator);
    }
    vkDestroySurfaceKHR(SGF::VulkanInstance, surface, SGF::VulkanAllocator);
    glfwDestroyWindow(window);
}

void onWindowResize(GLFWwindow* window, int width, int height) {
    SGF_Device* pDevice = (SGF_Device*)glfwGetWindowUserPointer(window);
    SGF_Window* pWindow = (SGF_Window*)&window[-offsetof(SGF_Window, window)];
}

void SGF_Window::bindDevice(SGF_Device& device) {
    SGF::info("device bound to window: ");
    //SGF_Device* pDevice = &device;
    //glfwSetWindowUserPointer(window, pDevice);
    pDevice = &device;
    assert(swapchain == VK_NULL_HANDLE);
    surfaceFormat = pickSurfaceFormat(pDevice->physical, surface, { VK_FORMAT_B8G8R8A8_SRGB, VK_COLORSPACE_SRGB_NONLINEAR_KHR });

    //depthFormat = pDevice->getSupportedFormat(POSSIBLE_DEPTH_FORMATS, ARRAY_SIZE(POSSIBLE_DEPTH_FORMATS), VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_IMAGE_TILING_OPTIMAL);
	
    //glfwSetWindowSizeCallback(window, onWindowResize);
    //createRenderPass();
    createSwapchain();
    //createResources();
}

void SGF_Window::onUpdate() {
    glfwPollEvents();
}

bool SGF_Window::shouldClose() {
    return glfwWindowShouldClose(window);
}