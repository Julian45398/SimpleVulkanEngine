#pragma once

#ifdef SGF_LOG_TERMINAL
#include <iostream>
#endif

#ifdef SGF_LOG_ALL
#define SGF_LOG_DEBUG
#define SGF_LOG_INFO
#define SGF_LOG_WARN
#define SGF_LOG_ERROR
#define SGF_LOG_FATAL
#elif defined(SGF_LOG_INFO)
#define SGF_LOG_WARN
#define SGF_LOG_ERROR
#define SGF_LOG_FATAL
#elif defined(SGF_LOG_WARN)
#define SGF_LOG_ERROR
#define SGF_LOG_FATAL
#endif
#if defined(SGF_LOG_ERROR)
#define SGF_LOG_FATAL
#endif

#if !defined(SGF_LOG_FILE) && defined(SGF_LOG_TO_FILE)
#define SGF_LOG_FILE "log.txt"
#endif

#if defined(SGF_LOG_FILE)
#include <fstream>
#endif

namespace SGF {
#ifdef SGF_LOG_FILE 
inline std::ofstream _LogFile;
#endif
template<typename T>
inline void print(T arg) {
#ifdef SGF_LOG_FILE
        _LogFile << arg;
#endif
#ifdef SGF_LOG_TERMINAL
        std::cout << arg;
#endif
}
template<typename T, typename...Args>
inline void print(T toPrint, Args... args) {
#ifdef SGF_LOG_FILE
        _LogFile << toPrint;
#endif
#ifdef SGF_LOG_TERMINAL
        std::cout << toPrint;
#endif
        print(args...);
}
template<typename...Args>
inline void println(Args... args) {
        print(args..., '\n');
}
template<typename...Args>
inline void debug(Args... args) {
#if defined(SGF_LOG_DEBUG)
        println("[DEBUG]: ", args...);
#endif
}
template<typename...Args>
inline void info(Args... args) {
#if defined(SGF_LOG_INFO)
#if defined(SGF_LOG_TERMINAL)
        std::cout << "\33[32m";
        println("[INFO]: ", args...);
        std::cout << "\33[0m";
#else
        println("[INFO]: ", args...);
#endif
#endif
}
template<typename...Args>
inline void warn(Args... args) {
#if defined(SGF_LOG_WARN)
#if defined(SGF_LOG_TERMINAL)
        std::cout << "\33[33m";
        println("[WARN]: ", args...);
        std::cout << "\33[0m";
#else
        println("[WARN]: ", args...);
#endif
#endif
}

template<typename...Args>
inline void error(Args... args) {
#if defined(SGF_LOG_ERROR)
#if defined(SGF_LOG_TERMINAL)
        std::cout << "\33[31m";
        println("[ERROR]: ", args...);
        std::cout << "\33[0m";
#else
        println("[ERROR]: ", args...);
#endif
#endif
}
template<typename...Args>
inline void fatal(Args... args) {
#if defined(SGF_LOG_FATAL)
#if defined(SGF_LOG_TERMINAL)
        std::cout << "\33[31m";
        println("[FATAL]: ", args...);
        std::cout << "\33[0m";
#else
        println("[FATAL]: ", args...);
#endif
#if defined(SGF_LOG_FILE)
        _LogFile.close();
#endif
        exit(1);
#endif
}
}