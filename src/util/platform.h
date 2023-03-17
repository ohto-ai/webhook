#pragma once
#ifndef _WEBHOOK_PLATFORM_H_
#define _WEBHOOK_PLATFORM_H_

#include <future>

class PlatformHelper
{
private:
    PlatformHelper() = default;
    PlatformHelper(const PlatformHelper &) = delete;
    PlatformHelper &operator=(const PlatformHelper &) = delete;

public:
    static PlatformHelper &getInstance();

    inline std::string executeCommand(std::string cmd) const;
    std::shared_future<std::string> executeCommandAsync(std::string cmd) const;
    inline std::string getPlatform() const;
    inline std::string getCpuInfo() const;
    int getTerminalWidth() const;
    int getTerminalHeight() const;
};

#endif // !_WEBHOOK_PLATFORM_H_