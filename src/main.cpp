#include "SimpleVulkanEditor.h"

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
	SVE::init(1280, 720);
	SimpleVulkanEditor* App = new SimpleVulkanEditor();
	App->run();
	delete App;
	SVE::terminate();

	return 0;
}