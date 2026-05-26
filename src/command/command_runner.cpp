#include "command_runner.h"

#include <spdlog/spdlog.h>
#include <sstream>
#include <cstdio>

#if defined(_WIN32)
#define popen _popen
#define pclose _pclose
#endif

namespace ohtoai {

std::string CommandRunner::run(const std::string& command,
                               std::chrono::milliseconds timeout)
{
    (void)timeout; // Timeout handled at caller level via wait_for on future
    return executePopen(command, max_output_size_);
}

std::shared_future<std::string> CommandRunner::runAsync(const std::string& command)
{
    return std::async(std::launch::async, [this, command]() {
        return executePopen(command, max_output_size_);
    }).share();
}

std::string CommandRunner::executePopen(const std::string& cmd, size_t max_output)
{
    auto f = popen(cmd.c_str(), "r");
    if (!f) {
        spdlog::error("Failed to execute command: {}", cmd);
        return {};
    }

    std::string result;
    result.reserve(std::min(max_output, size_t{4096}));

    char buf[4096];
    while (fgets(buf, sizeof(buf), f) != nullptr) {
        result.append(buf);
        if (result.size() > max_output) {
            spdlog::warn("Command output truncated at {} bytes", max_output);
            result.resize(max_output);
            break;
        }
    }

    int status = pclose(f);
    if (status != 0)
        spdlog::warn("Command exited with status {}: {}", status, cmd);

    return result;
}

} // namespace ohtoai

#if defined(_WIN32)
#undef popen
#undef pclose
#endif
