#include "SVE_Backend.h"

#include "SVE_Entrypoint.hpp"

#define SGF_LOG_ALL
#define SGF_LOG_TERMINAL
#include "Logger.hpp"

#include "SGF.hpp"

int main() {
	if (false) {
		SGF::init();
		{
			SGF_Window window("Hello world!", 600, 400, false);
			SGF_Device device = SGF_Device::Builder().bindWindow(&window).graphicQueues(1).computeQueues(0).transferQueues(1)
				.requireFeature(SGF_DEVICE_FEATURE_GEOMETRY_SHADER).build();
			if (device.isFeatureEnabled(SGF_DEVICE_FEATURE_GEOMETRY_SHADER)) {
				SGF::info("Geometry shader is enabled!");
			} else {
				SGF::info("Geometry shader is disabled!");
			}
			while (!window.shouldClose()) {
				window.onUpdate();
			}
		}
		SGF::terminate();
	}
	if (true) {
		shl::logDebug("Version: ", PROJECT_VERSION);
		SVE::init(1280, 720);
		SVE::run();
		SVE::terminate();
	}

	return 0;
}