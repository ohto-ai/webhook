#pragma once

#include "config_model.hpp"
#include <filesystem>
#include <string>
#include <vector>

namespace ohtoai {

class ConfigManager {
public:
    explicit ConfigManager(std::filesystem::path config_path);

    bool load();
    bool generateDefaultIfMissing();
    bool hasChanged();

    const WebhookConfigModal& get() const { return config_; }
    std::vector<std::string> validate() const;

private:
    std::filesystem::path config_path_;
    std::filesystem::file_time_type last_write_time_;
    WebhookConfigModal config_;
};

} // namespace ohtoai
