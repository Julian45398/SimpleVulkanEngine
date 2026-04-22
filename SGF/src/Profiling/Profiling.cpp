#include "Profiling.hpp"
#include <imgui.h>

namespace SGF {
	Profiler::ScopeProfiler::ScopeProfiler(const char* name, Profiler& profiler) : profiler(profiler) {
			result.name = name;
			result.time = 0.0;
		}
	Profiler::ScopeProfiler::~ScopeProfiler() {
		result.time = timer.currentMillis();
		profiler.results.push_back(result);
	}
	Profiler::ScopeProfiler Profiler::ProfileScope(const char* name) {
		return ScopeProfiler(name, *this);
	}
	void Profiler::DisplayResults() {
		ImGui::Begin("Profiler");
		for (const auto& result : results) {
			ImGui::Text("%s: %f ms", result.name, result.time);
		}
		ImGui::End();
		results.clear();
	}
}