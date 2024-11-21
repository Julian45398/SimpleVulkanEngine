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


SimpleVulkanEditor App;

int main() {
#ifdef RUN_IN_IDE 
	changeWorkingDirectory();
#endif
	shl::logInfo("Version: ", PROJECT_VERSION);
	App.init();
	App.run();
	App.terminate();

	return 0;
}