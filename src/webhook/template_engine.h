#pragma once

#include <inja/inja.hpp>
#include <nlohmann/json.hpp>
#include <cpp-httplib/httplib.h>
#include <string>
#include <mutex>
#include <unordered_map>

namespace ohtoai {

class TemplateEngine {
public:
    TemplateEngine();

    void setBasicData(const nlohmann::json& data);
    nlohmann::json createRenderData(const std::string& hook_name,
                                    const std::string& hook_command,
                                    const httplib::Request& req) const;

    void addCommandOutput(nlohmann::json& data, const std::string& output) const;
    std::string render(const std::string& content, const nlohmann::json& data);
    void clearCache();

private:
    void setupCallbacks();
    static std::string escapeHtml(const std::string& input);

    inja::Environment env_;
    nlohmann::json basic_data_;
    std::mutex cache_mutex_;
    std::unordered_map<std::string, inja::Template> template_cache_;
};

} // namespace ohtoai
