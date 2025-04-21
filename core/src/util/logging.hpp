#pragma once

#include <iostream>
#include <cstring>

#ifdef SVE_LOG_TO_FILE
#ifndef SVE_TARGET_LOG_FILE
#define SVE_TARGET_LOG_FILE "logs.txt"
#endif
#endif

#define SVE_LOG_LEVEL_DEBUG 0
#define SVE_LOG_LEVEL_INFO 1
#define SVE_LOG_LEVEL_WARN 2
#define SVE_LOG_LEVEL_ERROR 3

#if defined(SVE_LOG_ALL)
#define SVE_LOG_LEVEL SVE_LOG_LEVEL_DEBUG
#elif defined(SVE_LOG_INFO)
#define SVE_LOG_LEVEL SVE_LOG_LEVEL_INFO
#elif defined(SVE_LOG_WARN)
#define SVE_LOG_LEVEL SVE_LOG_LEVEL_WARN
#elif defined(SVE_LOG_ERROR)
#define SVE_LOG_LEVEL SVE_LOG_LEVEL_ERROR
#else
#error Please define a log level that needs to be logged
#endif

#define CHECK_LOG_LEVEL(X) SVE_LOG_LEVEL <= X

namespace SVE {
	template<typename T>
	static inline void print(T arg)
	{
		std::cout << arg;
	}

	template<typename T, typename...Args>
	static inline void print(T toPrint, Args... args)
	{
		std::cout << toPrint;
		print(args...);
	}

	template<typename...Args>
	static inline void println(Args... args)
	{
		print(args...);
		std::cout << '\n';
	}

	static inline const char* cutFilepath(const char* filepath) {
		size_t index = strlen(filepath);
		for (; index > 0; index--)
		{
			if (filepath[index - 1] == '\\' || filepath[index - 1] == '/')
				break;
		}
		return filepath + index;
	}

	template<typename... Args>
	static inline void logFatal(Args... args)
	{
#ifndef SVE_LOG_TO_FILE
		std::cout << "\033[31m[FATAL ERROR";
		println("]: [", args..., "]\033[0m");
		//std::cerr << e.what() << "\033[0m" << std::endl;
#endif
		exit(EXIT_FAILURE);
	}


	template<typename...Args>
	static inline void logError(Args... args)
	{
#if CHECK_LOG_LEVEL(SVE_LOG_LEVEL_ERROR)
		std::cout << "\033[31m[ERROR";
		println("]: [", args..., "]\033[0m");
#endif
	}

	template<typename...Args>
	static inline void logWarn(Args... args)
	{
#if CHECK_LOG_LEVEL(SVE_LOG_LEVEL_WARN)
		std::cout << "\033[33m[WARNING";
		println("]: [", args..., "]\033[0m");
#endif
	}

	template<typename...Args>
	static inline void logInfo(Args... args)
	{
#if CHECK_LOG_LEVEL(SVE_LOG_LEVEL_INFO)
		std::cout << "\033[32m[INFO";
		println("]: [", args..., "]\033[0m");
#endif
	}

	template<typename...Args>
	static inline void logDebug(Args... args)
	{
#if CHECK_LOG_LEVEL(SVE_LOG_LEVEL_DEBUG)
		std::cout << "[DEBUG";
		println("]: [", args..., ']');
#endif
	}
}