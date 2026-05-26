#pragma once

#include "config/config_manager.h"
#include "logging/logging_manager.h"
#include "server_manager.h"

#include <string>
#include <memory>
#include <filesystem>

namespace ohtoai {

class Application {
public:
    Application(int argc, char** argv);
    ~Application();

    ExitReason run();

private:
    bool initialize();
    void setupWorkingDirectory();
    void setupSignalHandlers();

    std::filesystem::path config_path_{"hook.json"};
    bool generate_config_if_missing_ = true;
    bool quit_after_config_generate_ = true;

    std::unique_ptr<ConfigManager> config_manager_;
    std::unique_ptr<LoggingManager> logging_manager_;
    std::unique_ptr<ServerManager> server_manager_;
};

} // namespace ohtoai
