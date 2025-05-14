#include "SVE_Backend.h"

#include "SVE_Entrypoint.hpp"

#include "SGF.hpp"

void onWindowClose(const SGF::WindowCloseEvent& event, char* listener) {
	SGF::info("Hello on window close called!", listener);
}

int main() {
	SGF::TestLayer layer(66);
	SGF::LayerStack::push(layer);
	char* helloWorld = "how are you today?";
	SGF::EventManager::addListener(onWindowClose, helloWorld);

	if (true) {
		SGF::init();
		{
			SGF::Window window("Hello world!", 600, 400, SGF::WINDOW_FLAG_RESIZABLE);
			SGF::Device device = SGF::Device::Builder().bindWindow(&window).graphicQueues(1).computeQueues(0).transferQueues(1)
				.requireFeature(SGF::DEVICE_FEATURE_GEOMETRY_SHADER).build();
			//SGF::WindowEvents.subscribe(onEvent, &window);
			VkFence windowFence = device.fence();
			VkSemaphore windowSemaphore = device.semaphore();
			VkQueue presentQueue = device.presentQueue();
			while (!window.shouldClose()) {
				window.onUpdate();
				//window.nextImage(nullptr, windowFence);
			}
		}
		SGF::terminate();
	}
	if (false) {
		shl::logDebug("Version: ", PROJECT_VERSION);
		SVE::init(1280, 720);
		SVE::run();
		SVE::terminate();
	}

	return 0;
}