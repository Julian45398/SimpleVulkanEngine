// vkl.cpp: Definiert den Einstiegspunkt für die Anwendung.
//

#define VKL_ENABLE_VALIDATION
#include "vkl.h"

#include <iostream>

VkBool32 VKAPI_PTR debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageTypes, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
	std::cout << pCallbackData->pMessage << std::endl;
	return VK_TRUE;
}

int main()
{
	const char* extensions[] = {
		VK_EXT_DEBUG_UTILS_EXTENSION_NAME
	};
	VkInstance instance = vkl::createInstance(VK_VERSION_1_0, 1, extensions, debugCallback);
	if (instance == VK_NULL_HANDLE) {
		std::cout << "failed to create vulkan instance" << std::endl;
	}
	else {
		std::cout << "created vulkan instance" << std::endl;
		return 0;
	}
	VkPhysicalDevice pdevice = vkl::getPhysicalDevice(instance, nullptr, 0, nullptr);
	return 0;
}
