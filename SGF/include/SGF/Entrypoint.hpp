#pragma once

#include "SGF_Core.hpp"

int main() {
	SGF::PreInit();
	SGF::Init();
	SGF::Setup();
	SGF::Run();
	SGF::Cleanup();
	SGF::Terminate();
	return 0;
}