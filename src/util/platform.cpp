#include "platform.h"
#include <sstream>
#include <filesystem>

#if defined(_WIN32)
#define popen _popen
#define pclose _pclose
#include <windows.h>
#elif defined(__linux__)
#include <sys/ioctl.h>
#include <unistd.h>
#include <limits.h>
#elif defined(__APPLE__)
#include <sys/ioctl.h>
#include <sys/sysctl.h>
#include <unistd.h>
#include <mach-o/dyld.h>
#endif

namespace ohtoai {

PlatformHelper &PlatformHelper::getInstance()
{
    static PlatformHelper instance;
    return instance;
}

std::string PlatformHelper::executeCommand(const std::string& cmd) const
{
    auto f = popen(cmd.c_str(), "r");
    std::stringstream display;
    if (f != nullptr)
    {
        char buf[1024];
        while (fgets(buf, sizeof(buf), f) != nullptr)
            display << buf;
        pclose(f);
    }
    return display.str();
}

std::shared_future<std::string> PlatformHelper::executeCommandAsync(const std::string& cmd) const
{
    return std::async(std::launch::async, [this, cmd]()
                      { return executeCommand(cmd); })
        .share();
}

std::string PlatformHelper::getPlatform() const
{
#ifdef _WIN32
    return "Windows";
#elif __linux__
    return "Linux";
#elif __APPLE__
    return "MacOS";
#else
    return "Unknown";
#endif
}

std::string PlatformHelper::getHomeDirectory() const
{
    std::string home_dir;
#ifdef _WIN32
    home_dir = getenv("USERPROFILE");
#elif __linux__
    home_dir = getenv("HOME");
#elif __APPLE__
    home_dir = getenv("HOME");
#endif
    return home_dir;
}

std::string PlatformHelper::getCpuInfo() const
{
#ifdef _WIN32
    SYSTEM_INFO sysinfo;
    GetNativeSystemInfo(&sysinfo);

    std::ostringstream oss;
    oss << "CPU Type: ";

    switch (sysinfo.wProcessorArchitecture) {
        case PROCESSOR_ARCHITECTURE_AMD64:
            oss << "AMD64";
            break;
        case PROCESSOR_ARCHITECTURE_MIPS:
            oss << "MIPS";
            break;
        case PROCESSOR_ARCHITECTURE_ARM:
            oss << "ARM";
            break;
        case PROCESSOR_ARCHITECTURE_IA64:
            oss << "IA64";
            break;
        case PROCESSOR_ARCHITECTURE_INTEL:
            oss << "INTEL";
            break;
        default:
            oss << "UNKNOWN";
    }

    oss << " " << sysinfo.dwNumberOfProcessors << " processors";
    return oss.str();
#elif __linux__
    return executeCommand("cat /proc/cpuinfo | grep 'model name' | cut -d: -f2 | sed 's/^ //g' | uniq");
#elif __APPLE__
    size_t size = 128;
    char cpu_brand[128]{};
    sysctlbyname("machdep.cpu.brand_string", &cpu_brand, &size, NULL, 0);
    return cpu_brand;
#endif
}

int PlatformHelper::getTerminalWidth() const
{
#ifdef _WIN32
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    return csbi.srWindow.Right - csbi.srWindow.Left + 1;
#else
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    return w.ws_col;
#endif
}

int PlatformHelper::getTerminalHeight() const
{
#ifdef _WIN32
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    return csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
#else
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    return w.ws_row;
#endif
}

template<typename T>
static std::string resolveExecutablePath(T path_func)
{
    std::string result;
#ifdef __linux__
    char buf[PATH_MAX];
    ssize_t len = ::readlink("/proc/self/exe", buf, sizeof(buf));
    if (len != -1) {
        buf[len] = '\0';
        result = path_func(buf);
    }
#elif defined(_WIN32)
    HMODULE hModule = GetModuleHandle(nullptr);
    if (hModule != nullptr) {
        char buf[MAX_PATH];
        DWORD len = GetModuleFileName(hModule, buf, MAX_PATH);
        if (len > 0) {
            result = path_func(buf);
        }
    }
#elif defined(__APPLE__)
    char buf[PATH_MAX];
    uint32_t bufsize = sizeof(buf);
    if (_NSGetExecutablePath(buf, &bufsize) == 0) {
        result = path_func(buf);
    }
#endif
    return result;
}

std::string PlatformHelper::getExecutablePath() const
{
    return resolveExecutablePath([](const char* buf) {
        return std::filesystem::canonical(buf).string();
    });
}

std::string PlatformHelper::getProgramDirectory() const
{
    return resolveExecutablePath([](const char* buf) {
        return std::filesystem::canonical(buf).parent_path().string();
    });
}

} // namespace ohtoai

#ifdef _WIN32
#undef popen
#undef pclose
#endif
