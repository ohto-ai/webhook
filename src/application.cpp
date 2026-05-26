#include "application.h"
#include "util/platform.h"
#include "version.hpp"

#include <spdlog/spdlog.h>
#include <spdlog/fmt/fmt.h>
#include <thread>

#if defined(_WIN32)
#include <windows.h>
#else
#include <csignal>
#endif

namespace ohtoai {

namespace {
    // Global atomic for signal handlers to communicate with the Application
    std::atomic<bool> g_shutdown_requested{false};
    std::atomic<bool> g_reload_requested{false};

#if defined(_WIN32)
    BOOL WINAPI consoleCtrlHandler(DWORD ctrl_type)
    {
        if (ctrl_type == CTRL_C_EVENT || ctrl_type == CTRL_BREAK_EVENT ||
            ctrl_type == CTRL_CLOSE_EVENT || ctrl_type == CTRL_SHUTDOWN_EVENT) {
            g_shutdown_requested.store(true);
            return TRUE;
        }
        return FALSE;
    }
#else
    void posixSignalHandler(int signal)
    {
        if (signal == SIGHUP) {
            g_reload_requested.store(true);
        } else {
            g_shutdown_requested.store(true);
        }
    }
#endif
} // anonymous namespace

Application::Application(int argc, char** argv)
{
    (void)argc;
    (void)argv;

    setupWorkingDirectory();
    setupSignalHandlers();
}

Application::~Application() = default;

bool Application::initialize()
{
    config_manager_ = std::make_unique<ConfigManager>(config_path_);

    if (!std::filesystem::exists(config_path_)) {
        if (!generate_config_if_missing_) {
            fmt::print(stderr, "Config file not found: {}\n", config_path_.string());
            return false;
        }

        if (!config_manager_->generateDefaultIfMissing())
            return false;

        if (quit_after_config_generate_)
            return true;
    }

    if (!config_manager_->load())
        return false;

    LogConfig log_cfg;
    log_cfg.console_level = config_manager_->get().log.console_level;
    log_cfg.file_level = config_manager_->get().log.file_level;
    log_cfg.file_path = config_manager_->get().log.file_path;
    log_cfg.global_level = config_manager_->get().log.global_level;

    logging_manager_ = std::make_unique<LoggingManager>(log_cfg);
    if (!logging_manager_->initialize())
        return false;

    return true;
}

ExitReason Application::run()
{
    if (!initialize())
        return ExitReason::Terminate;

    while (!g_shutdown_requested.load()) {
        server_manager_ = std::make_unique<ServerManager>(*config_manager_);
        auto reason = server_manager_->start();

        if (reason != ExitReason::Reload)
            return reason;

        if (g_shutdown_requested.load())
            return ExitReason::Finish;

        spdlog::info("Reloading configuration...");
        config_manager_->load();
        server_manager_.reset();
    }

    spdlog::info("Shutting down...");
    return ExitReason::Finish;
}

void Application::setupWorkingDirectory()
{
    auto program_path = std::filesystem::path(PlatformHelper::getInstance().getHomeDirectory())
                        / ".ohtoai" / VersionHelper::getInstance().AppName;

    if (!std::filesystem::exists(program_path))
        std::filesystem::create_directories(program_path);

    std::filesystem::current_path(program_path);
    program_dir_ = program_path;
}

void Application::setupSignalHandlers()
{
#if defined(_WIN32)
    SetConsoleCtrlHandler(consoleCtrlHandler, TRUE);
#else
    std::signal(SIGINT, posixSignalHandler);
    std::signal(SIGTERM, posixSignalHandler);
    std::signal(SIGHUP, posixSignalHandler);
#endif
}

} // namespace ohtoai
