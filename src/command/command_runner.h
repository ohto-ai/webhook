#pragma once

#include <string>
#include <chrono>
#include <future>

namespace ohtoai {

class CommandRunner {
public:
    CommandRunner() = default;

    std::string run(const std::string& command,
                    std::chrono::milliseconds timeout = std::chrono::milliseconds(0));

    std::shared_future<std::string> runAsync(const std::string& command);

    // Limits output to a maximum size for safety
    void setMaxOutputSize(size_t max_size) { max_output_size_ = max_size; }

private:
    static std::string executePopen(const std::string& cmd, size_t max_output);

    size_t max_output_size_ = 1024 * 1024; // 1 MB default max output
};

} // namespace ohtoai
