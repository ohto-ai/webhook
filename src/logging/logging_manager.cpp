#include "logging_manager.h"

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/fmt/bundled/color.h>
#include <spdlog/fmt/fmt.h>

namespace ohtoai {

LoggingManager::LoggingManager(const LogConfig& config)
    : config_(config)
{
}

LoggingManager::~LoggingManager()
{
    if (logger_) {
        logger_->flush();
    }
}

bool LoggingManager::initialize()
{
    try {
        auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        console_sink->set_level(spdlog::level::from_str(config_.console_level));

        auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(config_.file_path, false);
        file_sink->set_level(spdlog::level::from_str(config_.file_level));

        logger_ = std::make_shared<spdlog::logger>("webhook", spdlog::sinks_init_list({console_sink, file_sink}));
        spdlog::set_default_logger(logger_);
        spdlog::set_level(spdlog::level::from_str(config_.global_level));
        spdlog::flush_every(std::chrono::seconds(5));
        return true;
    } catch (const spdlog::spdlog_ex& ex) {
        fmt::print(stderr, "Log initialization failed: {}\n", ex.what());
        return false;
    }
}

} // namespace ohtoai
