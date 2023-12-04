#include <range/v3/range.hpp>
#include <range/v3/view.hpp>

#include "logger.hpp"

namespace logger
{
    bool SetLogFile(const char *path)
    {
        return std::filesystem::exists(const_cast<std::filesystem::path &>(logFile) = path);
    }

    void ExternalLog(LogLevel level, const char *content)
    {
#if USE_CXX_FORMAT
        Log(level, "{:s}", content);
#else
        Log(level, "%s", content);
#endif
    }

    std::string log_binary_data(const unsigned char *const src, int size, int chunk_size)
    {
        std::string content;
        auto v = std::vector<unsigned char>{src, src + size};
        const auto data_chunk = v | ranges::views::chunk(chunk_size);
        for (auto chunk : data_chunk)
        {
            fmt::format_to(std::back_inserter(content), "{:02X}\n", fmt::join(chunk, " "));
        }
        return content;
    }
}