#include "SVE_Backend.h"

#include "SVE_Entrypoint.hpp"

#include "SGF.hpp"

using namespace SGF;


void onEvent(SGF::WindowResizeEvent&, SGF::Device* device) {
	SGF::info("Hello world: ", device->getName());
}

int main() {
	if (true) {
		SGF::init();
		{
			SGF::Window window("Hello world!", 600, 400, SGF::WINDOW_FLAG_RESIZABLE);
			SGF::Device device = SGF::Device::Builder().bindWindow(&window).graphicQueues(1).computeQueues(0).transferQueues(1)
				.requireFeature(SGF::DEVICE_FEATURE_GEOMETRY_SHADER).build();
			
			//SGF::WindowEvents.subscribe(onEvent, &window);
			while (!window.shouldClose()) {
				window.onUpdate();
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