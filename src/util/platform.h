#pragma once

#include <future>
#include <string>

namespace ohtoai {

class PlatformHelper
{
public:
    static PlatformHelper &getInstance();

    std::string executeCommand(const std::string& cmd) const;
    std::shared_future<std::string> executeCommandAsync(const std::string& cmd) const;
    std::string getPlatform() const;
    std::string getCpuInfo() const;
    int getTerminalWidth() const;
    int getTerminalHeight() const;
    std::string getExecutablePath() const;
    std::string getProgramDirectory() const;
    std::string getHomeDirectory() const;

private:
    PlatformHelper() = default;
    PlatformHelper(const PlatformHelper &) = delete;
    PlatformHelper &operator=(const PlatformHelper &) = delete;

    template<typename... Args>
    std::string resolvePath(Args&&... args) const;
};

} // namespace ohtoai

// Backward compatibility
using PlatformHelper = ohtoai::PlatformHelper;
