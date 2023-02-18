#include <sstream>
#include <future>
#include <spdlog/spdlog.h>
#if defined(_WIN32)
#define popen _popen
#define pclose _pclose
#define WIN32_LEAN_AND_MEAN
#defiine NOMINMAX
#include <windows.h>
#elif defined(__linux__)
#include <sys/ioctl.h>
#elif defined(__APPLE__)
#include <sys/ioctl.h>
#endif

class PlatformHelper
{
private:
    PlatformHelper() {}
    PlatformHelper(const PlatformHelper &) = delete;
    PlatformHelper &operator=(const PlatformHelper &) = delete;

public:
    static PlatformHelper &getInstance()
    {
        static PlatformHelper instance;
        return instance;
    }

    inline std::string executeCommand(std::string cmd) const
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
        spdlog::debug("\rExecute {}=>\n{}", cmd, display.str());
        return display.str();
    }
    
    inline decltype(auto) executeCommandAsync(std::string cmd) const
    {
        auto shared_future =  std::async(std::launch::async, [this, cmd]() { return executeCommand(cmd); }).share();
        std::thread([shared_future] { shared_future.wait(); }).detach();
        return shared_future;
    }

    inline std::string getPlatform() const
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

    inline std::string getCpuInfo() const
    {
#ifdef _WIN32
        // Windows implementation
        SYSTEM_INFO sysinfo;
        GetSystemInfo(&sysinfo);
        return sysinfo.dwProcessorType;
#elif __linux__
        // Linux implementation
        return executeCommand("cat /proc/cpuinfo | grep 'model name' | cut -d: -f2 | sed 's/^ //g' | uniq");
#elif __APPLE__
        // MacOS implementation
        int mib[2] = {CTL_HW, HW_MODEL};
        char model[128];
        systeminfo | findstr / C : "Processor" systeminfo | findstr / C : "Processor" size_t len = sizeof(model);
        sysctl(mib, 2, &model, &len, NULL, 0);
        return model;
#endif
    }

    inline int getTerminalWidth() const
    {
#ifdef _WIN32
        return GetSystemMetrics(SM_CXSCREEN);
#else
        struct winsize w;
        ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
        return w.ws_col;
#endif
    }

    inline int getTerminalHeight() const
    {
#ifdef _WIN32
        return GetSystemMetrics(SM_CYSCREEN);
#else
        struct winsize w;
        ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
        return w.ws_row;
#endif
    }
};

#ifdef _WIN32
#undef popen
#undef pclose
#endif
