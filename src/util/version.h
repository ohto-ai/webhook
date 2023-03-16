#pragma once
#ifndef _WEBHOOK_VERSION_H_
#define _WEBHOOK_VERSION_H_

#include <string>

class VersionHelper
{
private:
    VersionHelper() {}
    VersionHelper(const VersionHelper &) = delete;
    VersionHelper &operator=(const VersionHelper &) = delete;
public:
    static VersionHelper &getInstance()
    {
        static VersionHelper instance;
        return instance;
    }

    const std::string AsciiBanner{R"(
 ██████╗ ██╗  ██╗████████╗ ██████╗        █████╗ ██╗
██╔═══██╗██║  ██║╚══██╔══╝██╔═══██╗      ██╔══██╗██║
██║   ██║███████║   ██║   ██║   ██║█████╗███████║██║
██║   ██║██╔══██║   ██║   ██║   ██║╚════╝██╔══██║██║
╚██████╔╝██║  ██║   ██║   ╚██████╔╝      ██║  ██║██║
 ╚═════╝ ╚═╝  ╚═╝   ╚═╝    ╚═════╝       ╚═╝  ╚═╝╚═╝
 )"};

    const std::string AppName = PROJECT_NAME;
    const std::string Version = BUILD_VERSION;
    const std::string CommitHash = COMMIT_HASH;
    const std::string CommitDate = COMMIT_DATE;
    const std::string BuildDate = __DATE__;
    const std::string BuildTime = __TIME__;
    const bool IsDevVersion = IS_DEV_VERSION;
};

#endif // !_WEBHOOK_VERSION_H_
