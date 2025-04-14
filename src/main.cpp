#include "SimpleVulkanEditor.h"


int main() {
	shl::logInfo("Version: ", PROJECT_VERSION);
	SVE::init(1280, 720);
	SimpleVulkanEditor* App = new SimpleVulkanEditor();
	App->run();
	delete App;
	SVE::terminate();

	return 0;
}