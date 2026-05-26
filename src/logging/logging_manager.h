#pragma once

#include <string>
#include <memory>
#include <vector>

namespace spdlog {
    class logger;
    namespace sinks {
        class sink;
    }
}

namespace ohtoai {

struct LogConfig {
    std::string console_level = "info";
    std::string file_level = "info";
    std::string file_path = "webhook.log";
    std::string global_level = "info";
};

class LoggingManager {
public:
    explicit LoggingManager(const LogConfig& config);
    ~LoggingManager();

    bool initialize();
    std::shared_ptr<spdlog::logger> getLogger() const { return logger_; }

private:
    LogConfig config_;
    std::shared_ptr<spdlog::logger> logger_;
};

} // namespace ohtoai
