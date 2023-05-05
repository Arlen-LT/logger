#include "logger.h"

namespace logger
{
#if not ENABLE_SLOG
    const std::filesystem::path logFile;
    bool SetLogFile(const char *path)
    {
        return std::filesystem::exists(const_cast<std::filesystem::path &>(logFile) = std::filesystem::u8path(path)); // TODO: change all string to u8string.
    }
#endif

    void ExternalLog(LogLevel level, const char *format)
    {
#if ENABLE_SLOG
        slog_tag(SLOG_TAG, level, format);
#else
        Log(level, format);
#endif
    }
} // namespace logger
