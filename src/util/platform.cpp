#include "platform.h"
#include <sstream>
#if defined(_WIN32)
#define popen _popen
#define pclose _pclose
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#elif defined(__linux__)
#include <sys/ioctl.h>
#include <unistd.h>
#elif defined(__APPLE__)
#include <sys/ioctl.h>
#include <sys/sysctl.h>
#include <unistd.h>
#include <mach-o/dyld.h>
#endif
#include <ghc/fs_std.hpp>

PlatformHelper &PlatformHelper::getInstance()
{
    static PlatformHelper instance;
    return instance;
}

std::string PlatformHelper::executeCommand(std::string cmd) const
{
    auto f = popen(cmd.c_str(), "r");
    std::stringstream display;
    if (f != nullptr)
    {
        char buf_ps[1024];
        while (fgets(buf_ps, sizeof(buf_ps), f) != nullptr)
            display << buf_ps;
        pclose(f);
    }
    return display.str();
}

std::shared_future<std::string> PlatformHelper::executeCommandAsync(std::string cmd) const
{
    auto shared_future = std::async(std::launch::async, [this, cmd]()
                                    { return executeCommand(cmd); })
                             .share();
    std::thread([shared_future]
                { shared_future.wait(); })
        .detach();
    return shared_future;
}

std::string PlatformHelper::getPlatform() const
{
// 获取运行平台
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

std::string PlatformHelper::getCpuInfo() const
{
#ifdef _WIN32
    // Windows implementation
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

    oss << " " << sysinfo.dwNumberOfProcessors << " processors " << " @" << (sysinfo.dwProcessorType >> 16)
        << "." << (sysinfo.dwProcessorType & 0xFFFF) << "GHz";
    return oss.str();
#elif __linux__
    // Linux implementation
    return executeCommand("cat /proc/cpuinfo | grep 'model name' | cut -d: -f2 | sed 's/^ //g' | uniq");
#elif __APPLE__
    // MacOS implementation
    // Get CPU brand string
    size_t size = 128;
    char cpu_brand[128]{};
    sysctlbyname("machdep.cpu.brand_string", &cpu_brand, &size, NULL, 0);
    return cpu_brand;
#endif
}

int PlatformHelper::getTerminalWidth() const
{
#ifdef _WIN32
    return GetSystemMetrics(SM_CXSCREEN);
#else
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    return w.ws_col;
#endif
}

int PlatformHelper::getTerminalHeight() const
{
#ifdef _WIN32
    return GetSystemMetrics(SM_CYSCREEN);
#else
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    return w.ws_row;
#endif
}

std::string PlatformHelper::getExecutablePath() const {
    std::string executablePath;

    #ifdef __linux__
        char buf[PATH_MAX];
        ssize_t len = ::readlink("/proc/self/exe", buf, sizeof(buf));
        if (len != -1) {
            buf[len] = '\0';
            executablePath = fs::canonical(buf).string();
        }
    #elif defined(_WIN32)
        HMODULE hModule = GetModuleHandle(nullptr);
        if (hModule != nullptr) {
            char buf[MAX_PATH];
            DWORD len = GetModuleFileName(hModule, buf, MAX_PATH);
            if (len > 0) {
                executablePath = fs::canonical(buf).string();
            }
        }
    #elif defined(__APPLE__)
        char buf[PATH_MAX];
        uint32_t bufsize = sizeof(buf);
        if (_NSGetExecutablePath(buf, &bufsize) == 0) {
            executablePath = fs::canonical(buf).string();
        }
    #endif

    return executablePath;
}

std::string PlatformHelper::getProgramDirectory() const {
    std::string executablePath = fs::current_path().string();

    #ifdef __linux__
        char buf[PATH_MAX];
        ssize_t len = ::readlink("/proc/self/exe", buf, sizeof(buf));
        if (len != -1) {
            buf[len] = '\0';
            executablePath = fs::canonical(buf).parent_path().string();
        }
    #elif defined(_WIN32)
        HMODULE hModule = GetModuleHandle(nullptr);
        if (hModule != nullptr) {
            char buf[MAX_PATH];
            DWORD len = GetModuleFileName(hModule, buf, MAX_PATH);
            if (len > 0) {
                executablePath = fs::canonical(buf).parent_path().string();
            }
        }
    #elif defined(__APPLE__)
        char buf[PATH_MAX];
        uint32_t bufsize = sizeof(buf);
        if (_NSGetExecutablePath(buf, &bufsize) == 0) {
            executablePath = fs::canonical(buf).parent_path().string();
        }
    #endif

    return executablePath;
}


#ifdef _WIN32
#undef popen
#undef pclose
#endif
