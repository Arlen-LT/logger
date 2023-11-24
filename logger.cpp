#include "logger.hpp"

namespace logger
{
    bool SetLogFile(const char *path)
    {
        return std::filesystem::exists(const_cast<std::filesystem::path &>(logFile) = path);
    }

    void ExternalLog(LogLevel level, const char *content)
    {
        Log(level, "%s", content);
    }
}