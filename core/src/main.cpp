
#include "SGF.hpp"
//#include "SVE_Backend.h"
//#include "SVE_Entrypoint.hpp"

void onWindowClose(const SGF::WindowCloseEvent& event, char* listener) {
	SGF::info("Hello on window close called!", listener);
}
struct WindowCreateInfo {
	const char* windowTitle;
	uint32_t width;
	uint32_t height;
	SGF::WindowCreateFlags windowFlags;
	uint32_t graphicsQueueCount;
	uint32_t computeQueueCount;
	uint32_t transferQueueCount;
	//const SGF::DeviceFeature* pDeviceFeatures;
	uint32_t featureCount;
};

template<size_t SIZE>
void setSomething(const float(&helloworld)[SIZE]) {
	for (uint32_t i = 0; i < SIZE; ++i) {
		SGF::info("hello: ", helloworld[i]);
	}
}
//void CreateWindowApp(const char* windowTitle, uint32_t width, uint32_t height, SGF::WindowCreateFlags windowFlags, const SGF::DeviceFeature* pGraphicsDeviceFeatures, uint32_t featureCount);
void SGF::PreInit() {
	//SGF::SetWindowTitle("Hello World from Preinit");
	//SGF::SetWindowFlags(SGF::WINDOW_FLAG_RESIZABLE);
	//SGF::SetDeviceGraphicsQueueCount(1);
	//SGF::SetDeviceTransferQueueCount(1);
	const SGF::DeviceFeatureFlags requiredFeatures[] = {
		SGF::DEVICE_FEATURE_GEOMETRY_SHADER
	};
	//SGF::SetDeviceRequiredFeatures(requiredFeatures, ARRAY_SIZE(requiredFeatures));
}

void SGF::Setup() {

}
void SGF::Cleanup() {

}

