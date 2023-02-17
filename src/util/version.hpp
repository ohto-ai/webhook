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
        VERSION_MINOR = 1,
        VERSION_PATCH = 1,
    };

public:
    static VersionHelper &getInstance()
    {
        static VersionHelper instance;
        return instance;
    }

    static constexpr const char *AsciiBanner[] =
        {
            " ██████╗ ██╗  ██╗████████╗ ██████╗        █████╗ ██╗",
            "██╔═══██╗██║  ██║╚══██╔══╝██╔═══██╗      ██╔══██╗██║",
            "██║   ██║███████║   ██║   ██║   ██║█████╗███████║██║",
            "██║   ██║██╔══██║   ██║   ██║   ██║╚════╝██╔══██║██║",
            "╚██████╔╝██║  ██║   ██║   ╚██████╔╝      ██║  ██║██║",
            " ╚═════╝ ╚═╝  ╚═╝   ╚═╝    ╚═════╝       ╚═╝  ╚═╝╚═╝"};

    const std::string Version = fmt::format("v{}.{}.{}", VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH);
};
