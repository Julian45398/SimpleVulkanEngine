// SimpleVulkanEngine.cpp: Definiert den Einstiegspunkt für die Anwendung.
//
#include "engine_core.h"

int main() {
	shl::logInfo("Version: ", PROJECT_VERSION);
	initWindow();
	while (!glfwWindowShouldClose(g_Window)) 
		glfwPollEvents();

	glfwDestroyWindow(g_Window);

	return 0;
}
