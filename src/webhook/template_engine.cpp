#include "template_engine.h"

#include <spdlog/spdlog.h>
#include <spdlog/fmt/fmt.h>
#include <fplus/fplus.hpp>
#include <algorithm>

namespace ohtoai {

TemplateEngine::TemplateEngine()
{
    setupCallbacks();
}

void TemplateEngine::setBasicData(const nlohmann::json& data)
{
    basic_data_ = data;
}

nlohmann::json TemplateEngine::createRenderData(const std::string& hook_name,
                                                 const std::string& hook_command,
                                                 const httplib::Request& req) const
{
    using nlohmann::literals::operator"" _json_pointer;

    nlohmann::json data = basic_data_;

    data["/context/name"_json_pointer] = hook_name;
    data["/context/command"_json_pointer] = hook_command;
    data["/request/method"_json_pointer] = req.method;
    data["/request/path"_json_pointer] = req.path;
    data["/request/body"_json_pointer] = req.body;
    data["/request/remote_addr"_json_pointer] = req.remote_addr;
    data["/request/remote_port"_json_pointer] = req.remote_port;

    for (const auto& [key, value] : req.headers)
        data["/request/header"_json_pointer][fplus::to_lower_case(key)] = value;

    data["/request/content_length"_json_pointer] = req.body.size();

    return data;
}

void TemplateEngine::addCommandOutput(nlohmann::json& data, const std::string& output) const
{
    using nlohmann::literals::operator"" _json_pointer;
    data["command_output"] = output;
    data["/command/output"_json_pointer] = output;
    data["/command/output_html"_json_pointer] = escapeHtml(output);
}

std::string TemplateEngine::render(const std::string& content, const nlohmann::json& data)
{
    // Check cache first
    {
        std::lock_guard<std::mutex> lock(cache_mutex_);
        auto it = template_cache_.find(content);
        if (it != template_cache_.end()) {
            return env_.render(it->second, data);
        }
    }

    // Parse and cache
    auto tmpl = env_.parse(content);

    {
        std::lock_guard<std::mutex> lock(cache_mutex_);
        template_cache_[content] = tmpl;
    }

    return env_.render(tmpl, data);
}

void TemplateEngine::clearCache()
{
    std::lock_guard<std::mutex> lock(cache_mutex_);
    template_cache_.clear();
}

void TemplateEngine::setupCallbacks()
{
    env_.add_void_callback("info", 0, [](inja::Arguments& args) {
        std::vector<std::string> result(args.size());
        std::transform(args.begin(), args.end(), result.begin(),
                       [](const nlohmann::json* j) { return j->get<std::string>(); });
        spdlog::info("{}", fmt::join(result, " "));
    });

    env_.add_void_callback("warn", 0, [](inja::Arguments& args) {
        std::vector<std::string> result(args.size());
        std::transform(args.begin(), args.end(), result.begin(),
                       [](const nlohmann::json* j) { return j->get<std::string>(); });
        spdlog::warn("{}", fmt::join(result, " "));
    });

    env_.add_void_callback("error", 0, [](inja::Arguments& args) {
        std::vector<std::string> result(args.size());
        std::transform(args.begin(), args.end(), result.begin(),
                       [](const nlohmann::json* j) { return j->get<std::string>(); });
        spdlog::error("{}", fmt::join(result, " "));
    });
}

std::string TemplateEngine::escapeHtml(const std::string& input)
{
    std::string escaped;
    escaped.reserve(input.size());
    for (char c : input) {
        switch (c) {
            case '&':  escaped += "&amp;"; break;
            case '<':  escaped += "&lt;"; break;
            case '>':  escaped += "&gt;"; break;
            case '"':  escaped += "&quot;"; break;
            case '\'': escaped += "&#39;"; break;
            default:   escaped += c; break;
        }
    }
    return escaped;
}

} // namespace ohtoai
