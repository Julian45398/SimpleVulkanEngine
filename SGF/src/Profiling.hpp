#pragma once

#include "SGF_Core.hpp"

namespace SGF {
	class Profiler {
	public:
		struct ProfileResult {
			const char* name;
			double time;
		};
		class ScopeProfiler {
		public:
			~ScopeProfiler();
		private:
			ScopeProfiler(const char* name, Profiler& profiler);
		private:
			friend Profiler;
			Timer timer;
			ProfileResult result;
			Profiler& profiler;
		};
		ScopeProfiler ProfileScope(const char* name);
		void DisplayResults();
	private:
		friend ScopeProfiler;
		std::vector<ProfileResult> results;

	};
}