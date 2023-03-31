#include "webhook_manager.h"
#include "util/platform.h"
#include "version.hpp"

#include <ghc/filesystem.hpp>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/fmt/bundled/color.h>
#include <spdlog/fmt/fmt.h>
#include <algorithm>
#include <cppcodec/base64_default_rfc4648.hpp>
#include <fplus/fplus.hpp>

int ohtoai::WebhookManager::exec()
{
    welcome(); // Print welcome message

    if (!serve_precondition())
        return -1;

    if (!doLoadConfig())
        return -1;

    if (!installLoggers())
        return -1;

    if (!installHooks())
        return -1;

    server.bind_to_port(config.listen.host.c_str(), config.listen.port);

    server.set_logger([](const httplib::Request &req, const httplib::Response &res)
                      {
                          spdlog::info("{} {} {} {} bytes User-Agent: {}", req.remote_addr, req.method, req.path, res.body.size(), req.get_header_value("User-Agent"));
                          for (const auto &header : req.headers)
                          {
                              spdlog::debug("Header: {}={}", header.first, header.second);
                          }
                          fmt::print(fg(fmt::color::green), "\r{:=^{}}\n", " Done ", PlatformHelper::getInstance().getTerminalWidth()); });

    server.set_pre_routing_handler([this](const auto& req, auto& res) { return authRoutingHandler(req, res);});

    spdlog::info("Server listen {}:{}.", config.listen.host, config.listen.port);

    return server.listen_after_bind() ? 0 : 1;
    // ~configurator.exitMonitorLoop();
}

void ohtoai::WebhookManager::welcome() const
{
    fmt::print(fg(fmt::color::gold), "{}\n", VersionHelper::getInstance().AsciiBanner);
    fmt::print(fg(fmt::color::green), "\r{:=^{}}\n", "=", PlatformHelper::getInstance().getTerminalWidth());
    if (VersionHelper::getInstance().IsDevVersion)
    {
        fmt::print(fg(fmt::color::red), "This is a development version, please do not use it in production environment.\n");
        fmt::print("Version {} on {}\n", VersionHelper::getInstance().CommitHash, VersionHelper::getInstance().CommitDate);
    }
    else
    {
        fmt::print("Version {}({}) on {}\n", VersionHelper::getInstance().Version, VersionHelper::getInstance().CommitHash, VersionHelper::getInstance().CommitDate);
    }
    fmt::print("Build on {} {}\n", VersionHelper::getInstance().BuildDate, VersionHelper::getInstance().BuildTime);
    fmt::print("Run on {} | {}\n", PlatformHelper::getInstance().getPlatform(), PlatformHelper::getInstance().getCpuInfo());
    fmt::print(fg(fmt::color::green), "\r{:=^{}}\n", "=", PlatformHelper::getInstance().getTerminalWidth());
}

bool ohtoai::WebhookManager::serve_precondition()
{
    if (!configurator.exists())
    {
        fmt::print("Config file not found, generate a new one.\n");
        nlohmann::json j = WebhookConfigModal::generate();
        configurator.assign(j);
        configurator.save();
        return false;
    }
    return true;
}

bool ohtoai::WebhookManager::doLoadConfig()
{
    // for test
    using nlohmann::literals::operator"" _json_pointer;
    configurator["/listen/port"_json_pointer].on_changed += [](FileConfigurator &configurator, const ConfigReference &ref, const nlohmann::json &diff)
    {
        fmt::print("Config item {} changed to {}.\n{}\n", ref.to_string(), configurator.get<int>(ref), diff.dump(4));
    };

    fmt::print("Load config {}\n", configurator.path());
    configurator.load();
    configurator.enterMonitorLoop();
    try
    {
        config = configurator.getJson().get<WebhookConfigModal>();
    }
    catch (std::exception e)
    {
        fmt::print(stderr, "{}\n", e.what());
        return false;
    }
    fmt::print("Config loaded.\n");
    return true;
}

bool ohtoai::WebhookManager::installLoggers()
{
    try
    {
        auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        console_sink->set_level(spdlog::level::from_str(config.log.console_level));

        auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(config.log.file_path, false);
        file_sink->set_level(spdlog::level::from_str(config.log.file_level));

        spdlog::set_default_logger(std::make_shared<spdlog::logger>("webhook", spdlog::sinks_init_list({console_sink, file_sink})));
        spdlog::set_level(spdlog::level::from_str(config.log.global_level));
        spdlog::flush_every(std::chrono::seconds(5));
    }
    catch (const spdlog::spdlog_ex &ex)
    {
        fmt::print(stderr, "Log initialization failed: {}\n", ex.what());
    }
    return true;
}

bool ohtoai::WebhookManager::installHooks()
{
    using nlohmann::literals::operator"" _json_pointer;

    for (const auto &hook : config.hooks)
    {
        auto path = fmt::format("{}{}", config.listen.prefix, hook.path);
        auto content = fmt::format("{}", fmt::join(hook.result.content, "\n"));
        spdlog::info("Bind `{}` {} {} hook, with command `{}`", hook.name, hook.method, path, hook.command);
        auto handler = [&, content](const httplib::Request &req, httplib::Response &res)
        {
            spdlog::info("Trigger hook `{}`", hook.name);

            auto env = injaEnv;
            auto data = basic_render_data;

            data["/context/name"_json_pointer] = hook.name;
            data["/context/command"_json_pointer] = hook.command;
            data["/request/method"_json_pointer] = req.method;
            data["/request/path"_json_pointer] = req.path;
            data["/request/body"_json_pointer] = req.body;
            data["/request/remote_addr"_json_pointer] = req.remote_addr;
            data["/request/remote_port"_json_pointer] = req.remote_port;
            for (const auto &[key, value] : req.headers)
                data["/request/header"_json_pointer][key] = value;
            data["/request/content_length"_json_pointer] = req.body.size();
            auto rendered_command = env.render(hook.command, data);
            data["/context/rendered_command"_json_pointer] = rendered_command;

            auto command_output_future = PlatformHelper::getInstance().executeCommandAsync(rendered_command);
            env.add_callback("command_output", 0, [&](inja::Arguments &args) -> nlohmann::json
                {
                if (hook.command_timeout > 0)
                {
                    if(command_output_future.wait_for(std::chrono::milliseconds(0)) != std::future_status::ready)
                    {
                        spdlog::info("Waiting for command output... (timeout: {}ms)", hook.command_timeout);
                    }
                    if (command_output_future.wait_for(std::chrono::milliseconds(hook.command_timeout)) == std::future_status::ready)
                    {
                        spdlog::info("Command output received");
                        return command_output_future.get();
                    }
                    else
                    {
                        spdlog::warn("Command output timeout");
                        return std::string{};
                    }
                }
                else
                {
                    spdlog::info("Waiting for command output...");
                    return command_output_future.get();
                } });
            // add try-catch
            try
            {
                auto result = env.render(content, data);
                res.set_content(result, hook.result.type.c_str());
                if (result.size() > 1024)
                    spdlog::debug("Render: \n\r{}...\n", result.substr(0, 1024));
                else
                    spdlog::debug("Render: \n\r{}\n", result);
            }
            catch (const std::exception &e)
            {
                spdlog::error("Render content failed: {}", e.what());
                res.status = 500;
                res.set_content("Render content failed", "text/plain");
                return;
            }
        };
        if (hook.method == "GET")
            server.Get(path.c_str(), handler);
        else if (hook.method == "POST")
            server.Post(path.c_str(), handler);
        else if (hook.method == "Delete")
            server.Delete(path.c_str(), handler);
        else if (hook.method == "Put")
            server.Put(path.c_str(), handler);
        else if (hook.method == "Options")
            server.Options(path.c_str(), handler);
        else if (hook.method == "Patch")
            server.Patch(path.c_str(), handler);
        else
            spdlog::error("Illegal method: {}", hook.method);
    }

    return true;
}

httplib::Server::HandlerResponse ohtoai::WebhookManager::authRoutingHandler(const httplib::Request &req, httplib::Response &res)
{
    if (req.has_header("X-Real-IP"))
    {
        const_cast<httplib::Request &>(req).remote_addr = req.get_header_value("X-Real-IP");
    }

    if (!config.listen.auth.path.empty() && !config.listen.auth.username.empty() && !config.listen.auth.password.empty() && fplus::is_prefix_of(config.listen.auth.path, req.path))
    {
        // verify username && password
        if (req.has_header("Authorization"))
        {
            std::string auth = req.get_header_value("Authorization");
            if (auth.find("Basic ") == 0)
            {
                auth = auth.substr(6);
                auth = base64::encode(auth);
                if (auth.find(":") != std::string::npos)
                {
                    std::string username = auth.substr(0, auth.find(":"));
                    std::string password = auth.substr(auth.find(":") + 1);
                    if (username == config.listen.auth.username && password == config.listen.auth.password)
                    {
                        return httplib::Server::HandlerResponse::Unhandled;
                    }
                }
            }
        }

        spdlog::warn("Auth failed");
        res.status = 401;
        res.set_header("WWW-Authenticate", "Basic realm=\"Webhook\"");
        res.set_content("Unauthorized", "text/plain");
        return httplib::Server::HandlerResponse::Handled;
    }
    return httplib::Server::HandlerResponse::Unhandled; 
}

ohtoai::WebhookManager::WebhookManager(int argc, char **argv) : configurator(ghc::filesystem::path(PlatformHelper::getInstance().getProgramDirectory()) / "hook.json") {
    ghc::filesystem::current_path(PlatformHelper::getInstance().getProgramDirectory());
    basic_render_data["/context/app"_json_pointer] = VersionHelper::getInstance().AppName;
    basic_render_data["/context/version"_json_pointer] = VersionHelper::getInstance().Version;
    basic_render_data["/context/commit_hash"_json_pointer] = VersionHelper::getInstance().CommitHash;
    basic_render_data["/context/commit_date"_json_pointer] = VersionHelper::getInstance().CommitDate;
    basic_render_data["/context/build_date"_json_pointer] = VersionHelper::getInstance().BuildDate;
    basic_render_data["/context/build_time"_json_pointer] = VersionHelper::getInstance().BuildTime;
    basic_render_data["/context/platform"_json_pointer] = PlatformHelper::getInstance().getPlatform();

    injaEnv.add_void_callback("info", [](inja::Arguments &args)
                              {
        std::vector<std::string> result(args.size());
        std::transform(args.begin(), args.end(), result.begin(),
                [](const nlohmann::json* j) { return j->get<std::string>(); });
        spdlog::info("{}", fmt::join(result, " ")); });
    injaEnv.add_void_callback("warn", [](inja::Arguments &args)
                              {
        std::vector<std::string> result(args.size());
        std::transform(args.begin(), args.end(), result.begin(),
                [](const nlohmann::json* j) { return j->get<std::string>(); });
        spdlog::warn("{}", fmt::join(result, " ")); });

    injaEnv.add_void_callback("error", [](inja::Arguments &args)
                              {
        std::vector<std::string> result(args.size());
        std::transform(args.begin(), args.end(), result.begin(),
                [](const nlohmann::json* j) { return j->get<std::string>(); });
        spdlog::error("{}", fmt::join(result, " ")); });
}

ohtoai::WebhookManager::~WebhookManager() {}
