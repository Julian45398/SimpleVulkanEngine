// SimpleVulkanEngine.cpp: Definiert den Einstiegspunkt für die Anwendung.
//
#include "engine_core.h"

#include "ui_data.h"
#include "render/ModelRenderer.h"
#include "render/ModelVertexBuffer.h"
#include "render/StagingBuffer.h"


#ifdef RUN_IN_IDE 
#ifdef _WIN32
#  include <direct.h>
#  define getcwd _getcwd
#  define chdir _chdir
#	define OUT_DIRECTORY "\\out\\"
#else
#	define OUT_DIRECTORY "/out/"
#  include <unistd.h>
#endif

#include <cstring>
#include <algorithm>
void changeWorkingDirectory() {
	char workingDirectoryPath[4096];
	getcwd(workingDirectoryPath, 4096);
	char* position = strstr(workingDirectoryPath, "\\out\\");
	if (position == nullptr) {
		shl::logWarn("failed to find the working directory");
	}
	else {
		position[0] = '\0';
	}
	chdir(workingDirectoryPath);
	shl::logInfo("Working Directory: ", workingDirectoryPath);
}
#endif


int main() {
#ifdef RUN_IN_IDE 
	changeWorkingDirectory();
#endif
	shl::logInfo("Version: ", PROJECT_VERSION);
	initBackend();
	Core.initialize(600, 400);
	ModelRenderer model_renderer;
	ModelVertexBuffer model_buffer;
	StagingBuffer staging_buffer;
	while (Core.shouldRun()) {
		glfwPollEvents();
		auto primary_commands = Core.beginRendering();
		model_renderer.recordCommands(Core.getNextSecondaryCommands(), primary_commands);
		controlUI();
		Core.finalizeRendering();
	}
	shl::logInfo("closed app");
	//Core.terminate();


	return 0;
}
