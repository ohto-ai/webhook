#include <spdlog/fmt/fmt.h>

#ifndef LATEST_GIT_TAG
#define LATEST_GIT_TAG "Origin"
#endif
#ifndef VERSION_TAG
#define VERSION_TAG "Dev_Origin"
#endif

constexpr const char *AsciiBanner[] =
    {
        " ██████╗ ██╗  ██╗████████╗ ██████╗        █████╗ ██╗",
        "██╔═══██╗██║  ██║╚══██╔══╝██╔═══██╗      ██╔══██╗██║",
        "██║   ██║███████║   ██║   ██║   ██║█████╗███████║██║",
        "██║   ██║██╔══██║   ██║   ██║   ██║╚════╝██╔══██║██║",
        "╚██████╔╝██║  ██║   ██║   ╚██████╔╝      ██║  ██║██║",
        " ╚═════╝ ╚═╝  ╚═╝   ╚═╝    ╚═════╝       ╚═╝  ╚═╝╚═╝"};

const auto VERSION_STRING = VERSION_TAG;
