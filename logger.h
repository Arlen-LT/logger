#pragma once

#include <mutex>
#include <iostream>
#include <ostream>
#include <fstream>
#include <filesystem>
#include <regex>

#if __cpp_lib_format
#include <format>
#endif

#if __cpp_lib_source_location
#include <source_location>
#endif

#if _WIN64
#define gettid() std::this_thread::get_id()
#elif __ANDROID__
#include <android/log.h>
#include <unistd.h>
#endif

#if _WIN64
#define LOGGER_EXPORT __declspec(dllexport)
#else
#define LOGGER_EXPORT
#endif

namespace logger
{
#if not ENABLE_SLOG
    enum class LogLevel : const char
    {
        Trace = 'T',
        Debug = 'D',
        Info = 'I',
        Warning = 'W',
        Error = 'E',
        Fatal = 'F'
    };

    extern const std::filesystem::path logFile;

    extern "C" LOGGER_EXPORT bool SetLogFile(const char *path);

    template <typename... Args>
    struct Log
    {
        Log(LogLevel level, const char *format, Args &&...args
#if __cpp_lib_source_location
            ,
            const std::source_location &location = std::source_location::current()
#endif
        )
        {
            std::ofstream logStream{logFile, std::ios::out | std::ios::app};
            std::ostream *os = logStream.is_open() ? &logStream : &std::cout;
            time_t current = std::time(nullptr);
            *os << std::left
                << std::put_time(std::localtime(&current), "%F %T ")
                << "[" << static_cast<char>(level) << "]"
                << "[thread " << gettid() << "] ";
#if __cpp_lib_source_location
            *os << location.file_name()
                << "(" << location.line() << "," << location.column() << ","
                << std::quoted(location.function_name())
                << "): ";
#endif
#if __cpp_lib_format
            *os << std::vformat(std::regex_replace(format, std::regex(R"(%(?:\d+\$)?[-+0 #]*\d*(?:\.\d+)?[hljztL]*[diouxXfeEgGaAcspn])"), "{}")
                , std::make_format_args(args...)) << "\n";
#else
            std::string_view sv{format};
            if constexpr (sizeof...(Args) == 0)
                *os << sv;
            else
            {

                auto TryInsert = [&sv, &os]<typename T>(size_t n, T obj)
                {
                    for (auto it = sv.begin(); it != sv.end(); ++it)
                    {
                        if (*it == '%')
                        {
                            if constexpr (std::is_null_pointer_v<T>)
                                *os << "(compile-time nullptr)";
                            else if constexpr (std::is_pointer_v<T>)
                                obj == nullptr ? *os << "(runtime nullptr)" : *os << obj;
                            else
                                *os << obj;
                            it += 2;
                            break;
                        }
                        else
                        {
                            *os << *it;
                        }
                    }
                };

                [&]<std::size_t... N>(std::index_sequence<N...>)
                {
                    ((TryInsert(N, std::get<N>(std::forward_as_tuple(args...)))), ...);
                }(std::index_sequence_for<Args...>{});
            }

            *os << "\n";
#endif // ENABLE_STD_FORMAT
        }
    };

    template <typename... Args>
    Log(LogLevel, const char *, Args &&...) -> Log<Args...>;
#else // ENABLE_SLOG
#ifndef SLOG_TAG
#define SLOG_TAG __FILE__
#endif
#include "slog.h"
    enum LogLevel
    {
        Trace = L_TRACE,
        Debug = L_DEBUG,
        Info = L_INFO,
        Warning = L_WARN,
        Error = L_ERR,
        Fatal = L_FATAL
    };
#define Log(x, y, ...)                                \
    do                                                \
    {                                                 \
        slog_tag(SLOG_TAG, x, y "\n", ##__VA_ARGS__); \
    } while (0)
#endif // ENABLE_SLOG

    extern "C" LOGGER_EXPORT void ExternalLog(LogLevel level, const char *format);
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
