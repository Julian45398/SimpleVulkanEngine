#include "SimpleVulkanEditor.h"

#include <filesystem>

int main() {

	std::filesystem::path currentPath = std::filesystem::current_path();
	shl::logDebug("Launching in: ", currentPath);
	shl::logDebug("Version: ", PROJECT_VERSION);
	SVE::init(1280, 720);
	SimpleVulkanEditor* App = new SimpleVulkanEditor();
	App->run();
	delete App;
	SVE::terminate();

	return 0;
}