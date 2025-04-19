#include "SVE_Backend.h"
#include "core/SGF.hpp"

#include "SVE_Entrypoint.hpp"

int main() {
	SGF::init();
	SGF::terminate();
	//std::filesystem::path currentPath = std::filesystem::current_path();
	//shl::logDebug("Launching in: ", currentPath);
	shl::logDebug("Version: ", PROJECT_VERSION);
	SVE::init(1280, 720);
	SVE::run();
	SVE::terminate();

	return 0;
}