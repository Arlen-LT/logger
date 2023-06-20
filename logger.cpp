#include "logger.h"

namespace logger
{
#ifndef ENABLE_SLOG
    const std::filesystem::path logFile;
    bool SetLogFile(const char *path)
    {
        return std::filesystem::exists(const_cast<std::filesystem::path &>(logFile) = std::filesystem::u8path(path)); // TODO: change all string to u8string.
    }
#endif

    void ExternalLog(LogLevel level, const char *content)
    {
#ifdef ENABLE_SLOG
        slog_tag(SLOG_TAG, level, "%s\n", content);
#else
        Log(level, "%s", content);
#endif
    }
} // namespace logger
