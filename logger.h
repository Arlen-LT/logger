#pragma once

#include <thread>
#include <mutex>
#include <iostream>
#include <ostream>
#include <fstream>
#include <filesystem>
#include <regex>
#include <functional>
#include <concepts>

#include <fmt/format.h>
#include <fmt/chrono.h>
#include <fmt/std.h>
#include <fmt/printf.h>

#if __cpp_lib_source_location
#include <source_location>
#endif

#if _WIN64
#define gettid() std::this_thread::get_id()
#else
#include <unistd.h>
#endif

#if _WIN64
#define LOGGER_EXPORT __declspec(dllexport)
#else
#define LOGGER_EXPORT
#endif

#if ENABLE_SLOG
#include "slog.h"
#else
enum { L_FATAL, L_ERR, L_WARN, L_INFO, L_STATUS, L_V, L_DEBUG, L_TRACE, L_STATS, L_MAX = L_STATS };
static inline void slog_tag(const char* tag, int lev, const char* format, ...) {}
#endif

namespace logger
{
    enum class LogLevel : int
    {
        Trace = L_TRACE,
        Debug = L_DEBUG,
        Info = L_INFO,
        Warning = L_WARN,
        Error = L_ERR,
        Fatal = L_FATAL
    };

    extern "C" LOGGER_EXPORT bool SetLogFile(const char* path);
    extern "C" LOGGER_EXPORT void ExternalLog(LogLevel level, const char* format);

    namespace {
        const std::filesystem::path logFile;
#if ENABLE_SLOG
        bool enable_slog = true;
#else
        bool enable_slog = false;
#endif
    }

    template <typename... Args>
    struct Log
    {
        Log(LogLevel level, const char* format, Args &&...args
#if __cpp_lib_source_location
            , const std::source_location& location = std::source_location::current()
#endif
        )
        {
            if (enable_slog)
            {
                slog_tag(SLOG_TAG, static_cast<int>(level), format, args...);
                return;
            }

            std::string content;
            std::time_t t = std::time(nullptr);
            fmt::format_to(std::back_inserter(content), "{:%Y-%m-%d %H:%M:%S} [{}][{}] ", fmt::localtime(t), static_cast<int>(level), gettid());
#if __cpp_lib_source_location
            fmt::format_to(std::back_inserter(content), "{}({},{}): ", location.file_name(), location.line(), location.function_name());
#endif
#if USE_CXX_FORMAT
            fmt::vformat_to(std::back_inserter(content), format, fmt::make_format_args(args...));
#else
            content += fmt::sprintf(format, args...);
#endif
            if (logFile.empty())
            {
                fmt::print("{}\n", content);
                return;
            }

            std::ofstream logStream{ logFile, std::ios::out | std::ios::app };
            if (logStream.is_open())
            {
                fmt::print(logStream, "{}\n", content);
                logStream.close();
            }
        }
    };

    template <typename... Args>
    Log(LogLevel, const char*, Args &&...) -> Log<Args...>;
}
using namespace logger;

#if DEBUG
#define CHECK_VARIABLE(x, y)                \
    do                                      \
    {                                       \
        Log(LogLevel::Debug, #x ": " y, x); \
    } while (0);

#define TRACE(x)                 \
    do                           \
    {                            \
        Log(LogLevel::Trace, x); \
    } while (0);
#else
#define CHECK_VARIABLE(x, y) ((void)0)
#define TRACE(x) ((void)0)
#endif
#undef LOGGER_EXPORT
