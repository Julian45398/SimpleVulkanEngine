#pragma once

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

#include <fmt/format.h>
#include <fmt/color.h>
#include <fmt/ranges.h>
#include <cstdlib>
#include <utility>

namespace SGF {
    namespace Log {
        // Generic print helpers (fmt::format_string provides compile-time format checks)
        template<typename... Args>
        inline void Print(fmt::format_string<Args...> fmtStr, Args&&... args) {
            fmt::print(fmtStr, std::forward<Args>(args)...);
        }
        template<typename... Args>
        inline void Println(fmt::format_string<Args...> fmtStr, Args&&... args) {
            fmt::println(fmtStr, std::forward<Args>(args)...);
        }

        template<typename T>
        inline void Debug(T arg) {
//#if defined(SGF_LOG_DEBUG)
            fmt::print("[DEBUG]: ");
            fmt::println("{}", arg);
//#endif
        }

        // Log-level functions � use fmt format strings
        template<typename... Args>
        inline void Debug(fmt::format_string<Args...> fmtStr, Args&&... args) {
//#if defined(SGF_LOG_DEBUG)
            fmt::print("[DEBUG]: ");
            fmt::println(fmtStr, std::forward<Args>(args)...);
//#endif
        }

        inline void Info(const char* arg) {
//#if defined(SGF_LOG_INFO)
			fmt::print(fg(fmt::color::green), "[INFO]: ");
            fmt::println("{}", arg);
//#endif
        }
        template<typename... Args>
        inline void Info(fmt::format_string<Args...> fmtStr, Args&&... args) {
//#if defined(SGF_LOG_INFO)
            fmt::print(fg(fmt::color::green), "[INFO]: ");
            fmt::println(fmtStr, std::forward<Args>(args)...);
//#endif
        }

        inline void Warn(const char* arg) {
//#if defined(SGF_LOG_WARN)
            fmt::print(fg(fmt::color::yellow), "[WARN]: ");
            fmt::println("{}", arg);
//#endif
        }
        template<typename... Args>
        inline void Warn(fmt::format_string<Args...> fmtStr, Args&&... args) {
//#if defined(SGF_LOG_WARN)
            fmt::print(fg(fmt::color::yellow), "[WARN]: ");
            fmt::println(fmtStr, std::forward<Args>(args)...);
//#endif
        }

        inline void Error(const char* arg) {
//#if defined(SGF_LOG_ERROR)
            fmt::print(fg(fmt::color::orange), "[ERROR]: ");
            fmt::println("{}", arg);
#if defined(SGF_STOP_ON_ERROR)
            std::exit(EXIT_FAILURE);
#endif
//#endif
        }
        template<typename... Args>
        inline void Error(fmt::format_string<Args...> fmtStr, Args&&... args) {
#if defined(SGF_LOG_ERROR)
            fmt::print(fg(fmt::color::orange), "[ERROR]: ");
            fmt::println(fmtStr, std::forward<Args>(args)...);
#if defined(SGF_STOP_ON_ERROR)
            std::exit(EXIT_FAILURE);
#endif
#endif
        }


        // Fatal always prints and exits
        template<typename T>
        [[noreturn]] inline void Fatal(T arg) {
            fmt::print(fg(fmt::color::red), "[FATAL]: ");
            fmt::println("{}", arg);
            std::exit(EXIT_FAILURE);
        }

        // Fatal always prints and exits
        template<typename... Args>
        [[noreturn]] inline void Fatal(fmt::format_string<Args...> fmtStr, Args&&... args) {
            fmt::print(fg(fmt::color::red), "[FATAL]: ");
            fmt::println(fmtStr, std::forward<Args>(args)...);
            std::exit(EXIT_FAILURE);
        }
    } // namespace Log

    // Optional convenience macros that avoid evaluating arguments when the level is disabled.
    // Use SGF_DEBUG(...), SGF_INFO(...), SGF_WARN(...), SGF_ERROR(...)
#ifdef SGF_LOG_DEBUG
#define SGF_DEBUG(...) SGF::Log::Debug(__VA_ARGS__)
#else
#define SGF_DEBUG(...) ((void)0)
#endif

#ifdef SGF_LOG_INFO
#define SGF_INFO(...) SGF::Log::Info(__VA_ARGS__)
#else
#define SGF_INFO(...) ((void)0)
#endif

#ifdef SGF_LOG_WARN
#define SGF_WARN(...) SGF::Log::Warn(__VA_ARGS__)
#else
#define SGF_WARN(...) ((void)0)
#endif

#ifdef SGF_LOG_ERROR
#define SGF_ERROR_MSG(...) SGF::Log::Error(__VA_ARGS__)
#else
#define SGF_ERROR_MSG(...) ((void)0)
#endif

#define SGF_FATAL(...) SGF::Log::Fatal(__VA_ARGS__)
} // namespace SGF