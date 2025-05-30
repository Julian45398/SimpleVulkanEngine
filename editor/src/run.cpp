#include "SimpleVulkanEditor.h"


namespace SVE {
	void run() {
		SimpleVulkanEditor* App = new SimpleVulkanEditor();
		App->run();
		delete App;
	}
}