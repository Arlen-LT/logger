#pragma once

#include <thread>
#include <iostream>
#include <fstream>
#include <filesystem>

#include <fmt/core.h>
#include <fmt/ranges.h>
#include <fmt/format.h>
#include <fmt/chrono.h>
#include <fmt/std.h>
#include <fmt/printf.h>

#if __cpp_lib_source_location
#include <source_location>
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
    std::string log_binary_data(const unsigned char *const src, int size, int chunk_size);

    namespace {
        const std::filesystem::path logFile;
    }

    template <typename... Args>
    struct LOGGER_EXPORT Log
    {
        Log(LogLevel level, const char* format, Args &&...args
#if __cpp_lib_source_location
            , const std::source_location& location = std::source_location::current()
#endif
        )
        {
#if ENABLE_SLOG
#if USE_CXX_FORMAT
            slog_tag(SLOG_TAG, static_cast<int>(level), "%s\n", fmt::vformat(format, fmt::make_format_args(args...)).c_str());
#else
            slog_tag(SLOG_TAG, static_cast<int>(level), "%s\n", fmt::sprintf(format, args...).c_str());
#endif
            return;
#endif

            std::string content;
            std::time_t t = std::time(nullptr);
            fmt::format_to(std::back_inserter(content), "{:%Y-%m-%d %H:%M:%S} [{}][{}] ", fmt::localtime(t), static_cast<int>(level), std::this_thread::get_id());
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
                fmt::print("{:s}\n", content);
                return;
            }

            std::ofstream logStream{logFile, std::ios::out | std::ios::app};
            if (logStream.is_open())
            {
                fmt::print(logStream, "{:s}\n", content);
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
