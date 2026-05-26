#include "config_manager.h"
#include <spdlog/spdlog.h>
#include <nlohmann/json.hpp>
#include <fstream>

namespace ohtoai {

ConfigManager::ConfigManager(std::filesystem::path config_path)
    : config_path_(std::move(config_path))
{
}

bool ConfigManager::generateDefaultIfMissing()
{
    if (std::filesystem::exists(config_path_))
        return true;

    spdlog::warn("Config file not found, generating default at {}", config_path_.string());

    nlohmann::json j = WebhookConfigModal::generate();
    std::ofstream ofs(config_path_);
    if (!ofs) {
        spdlog::error("Failed to create config file at {}", config_path_.string());
        return false;
    }
    ofs << j.dump(4) << std::endl;
    return true;
}

bool ConfigManager::load()
{
    spdlog::info("Loading config from {}", config_path_.string());

    try {
        last_write_time_ = std::filesystem::last_write_time(config_path_);
        nlohmann::json j;
        std::ifstream ifs(config_path_);
        if (!ifs) {
            spdlog::error("Failed to open config file {}", config_path_.string());
            return false;
        }
        ifs >> j;
        config_ = j;
    } catch (const std::exception& e) {
        spdlog::error("Config load error: {}", e.what());
        return false;
    }

    auto errors = validate();
    for (const auto& err : errors) {
        spdlog::warn("Config validation: {}", err);
    }

    spdlog::info("Config loaded ({} hooks, listen {}:{})",
                 config_.hooks.size(), config_.listen.host, config_.listen.port);
    return true;
}

bool ConfigManager::hasChanged()
{
    if (!std::filesystem::exists(config_path_))
        return false;

    auto current_time = std::filesystem::last_write_time(config_path_);
    if (current_time != last_write_time_) {
        last_write_time_ = current_time;
        return true;
    }
    return false;
}

std::vector<std::string> ConfigManager::validate() const
{
    std::vector<std::string> errors;

    if (config_.listen.port < 1 || config_.listen.port > 65535)
        errors.push_back("listen.port must be between 1 and 65535");

    for (size_t i = 0; i < config_.hooks.size(); ++i) {
        const auto& hook = config_.hooks[i];
        if (hook.path.empty())
            errors.push_back("hooks[" + std::to_string(i) + "].path is required");
        if (hook.command.empty())
            errors.push_back("hooks[" + std::to_string(i) + "].command is required");
        if (hook.method != "GET" && hook.method != "POST" && hook.method != "PUT" &&
            hook.method != "DELETE" && hook.method != "PATCH" && hook.method != "OPTIONS")
            errors.push_back("hooks[" + std::to_string(i) + "].method '" + hook.method + "' is not valid");
    }

    return errors;
}

} // namespace ohtoai
