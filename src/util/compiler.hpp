#ifndef APPNAME
#define APPNAME "webhook"
#endif
#ifndef CODE_VERSION
#define CODE_VERSION "0000000"
#endif
#ifndef CODE_DATE
#define CODE_DATE "Fri Nov 18 00:00:00 2022 +0800"
#endif
#ifndef BUILD_MACHINE_INFO
#define BUILD_MACHINE_INFO "Linux 5.4.0-121-generic x86_64"
#endif
#ifndef BUILD_MACHINE_FULL_INFO
#define BUILD_MACHINE_FULL_INFO "Linux ohtoai.top 5.4.0-121-generic #137-Ubuntu SMP Wed Jun 15 13:33:07 UTC 2022 x86_64 x86_64 x86_64 GNU/Linux"
#endif

class CompilerHelper
{
private:
    CompilerHelper() {}
    CompilerHelper(const CompilerHelper &) = delete;
    CompilerHelper &operator=(const CompilerHelper &) = delete;

public:
    static CompilerHelper &getInstance()
    {
        static CompilerHelper instance;
        return instance;
    }

public:
    const char *AppName = APPNAME;
    const char *CommitHash = COMMIT_HASH;
    const char *CommitDate = COMMIT_DATE;
    const char *BuildMachineInfo = BUILD_MACHINE_INFO;
    const char *BuildMachineFullInfo = BUILD_MACHINE_FULL_INFO;
    const char *BuildDate = __DATE__;
    const char *BuildTime = __TIME__;
};
