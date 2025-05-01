#include "SVE_Backend.h"

#include "SVE_Entrypoint.hpp"

#include "SGF.hpp"

int main() {
	if (true) {
		SGF::init();
		{
			SGF::Window window("Hello world!", 600, 400, false);
			SGF::Device device = SGF::Device::Builder().bindWindow(&window).graphicQueues(1).computeQueues(0).transferQueues(1)
				.requireFeature(SGF::DEVICE_FEATURE_GEOMETRY_SHADER).build();
			if (device.isFeatureEnabled(SGF::DEVICE_FEATURE_GEOMETRY_SHADER)) {
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