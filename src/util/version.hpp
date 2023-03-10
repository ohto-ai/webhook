#include <spdlog/fmt/fmt.h>

class VersionHelper
{
private:
    VersionHelper() {}
    VersionHelper(const VersionHelper &) = delete;
    VersionHelper &operator=(const VersionHelper &) = delete;

    enum
    {
        VERSION_MAJOR = 0,
        VERSION_MINOR = 2,
        VERSION_PATCH = 6,
    };

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

    const std::string Version = fmt::format("v{}.{}.{}", VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH);
};
