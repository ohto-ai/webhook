#include <spdlog/fmt/fmt.h>

// version
enum{
    VERSION_MAJOR = 0,
    VERSION_MINOR = 0,
    VERSION_PATCH = 1,
};

constexpr const char *AsciiBanner[] =
    {
        " ██████╗ ██╗  ██╗████████╗ ██████╗        █████╗ ██╗",
        "██╔═══██╗██║  ██║╚══██╔══╝██╔═══██╗      ██╔══██╗██║",
        "██║   ██║███████║   ██║   ██║   ██║█████╗███████║██║",
        "██║   ██║██╔══██║   ██║   ██║   ██║╚════╝██╔══██║██║",
        "╚██████╔╝██║  ██║   ██║   ╚██████╔╝      ██║  ██║██║",
        " ╚═════╝ ╚═╝  ╚═╝   ╚═╝    ╚═════╝       ╚═╝  ╚═╝╚═╝"};

const auto VERSION_STRING = fmt::format(FMT_STRING("v{}.{}.{}"), VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH);