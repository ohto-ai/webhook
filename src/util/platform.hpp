#include <sstream>

#ifdef _WIN32
#define popen _popen
#define pclose _pclose
#include <windows.h>
#elif __linux__
#elif __APPLE__
#include <sys/sysctl.h>
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

    inline std::string executeCommand(std::string cmd)
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

    inline std::string getPlatform()
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

    inline std::string getCpuInfo()
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
};

#ifdef _WIN32
#undef popen
#undef pclose
#endif