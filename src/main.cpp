#include "util/platform.h"
#include "util/version.h"
#include "config/config_modal.hpp"
#include "config/file_configurator.hpp"

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#endif

#include <ghc/fs_std.hpp>
#include <cpp-httplib/httplib.h>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/fmt/bundled/color.h>
#include <spdlog/fmt/fmt.h>
#include <mustache/mustache.hpp>
#include <algorithm>
#include <brynet/base/crypto/Base64.hpp>
#include <fplus/fplus.hpp>

int main(int argc, char **argv)
{
    constexpr auto configPath{"hook.json"};

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
    fmt::print("Run on {} | {}\n", PlatformHelper::getInstance().getPlatform() ,PlatformHelper::getInstance().getCpuInfo());
    fmt::print(fg(fmt::color::green), "\r{:=^{}}\n", "=", PlatformHelper::getInstance().getTerminalWidth());

    if (!fs::exists(configPath))
    {
        fmt::print("Config file not found, generate a new one.\n");
        WebhookConfigModal::generate(configPath);
        return 0;
    }

    FileConfigurator configurator(configPath);

    auto& itemListenPort = configurator.addConfigItem(nlohmann::json::json_pointer("/listen/port"));

    itemListenPort.on_changed.push_back([](FileConfigurator& configurator, const ConfigItemRef::reference_t& ref) {
        fmt::print("Config item {} changed to {}.\n", ref.to_string(), configurator.get<int>(ref));
    });

    fmt::print("Load config {}\n", configPath);
    configurator.load();
    configurator.enterMonitorLoop();
    WebhookConfigModal config;
    try
    {
        config = configurator.getJson().get<WebhookConfigModal>();
    }
    catch (std::exception e)
    {
        fmt::print(stderr, "{}\n", e.what());
        return -1;
    }
    fmt::print("Config loaded.\n");

    // Init log
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

    httplib::Server server;
    server.bind_to_port(config.listen.host.c_str(), config.listen.port);

    server.set_logger([](const httplib::Request &req, const httplib::Response &res)
                      {
                        spdlog::info("{} {} {} {} bytes User-Agent: {}", req.remote_addr, req.method, req.path, res.body.size(), req.get_header_value("User-Agent"));
                        for (const auto &header : req.headers)
                        {
                            spdlog::debug("Header: {}={}", header.first, header.second);
                        }
                        fmt::print(fg(fmt::color::green), "\r{:=^{}}\n", " Done ", PlatformHelper::getInstance().getTerminalWidth()); });

    server.set_pre_routing_handler([&config](const httplib::Request &req, httplib::Response &res)
                                   {
                                        if (req.has_header("X-Real-IP"))
                                        {
                                            const_cast<httplib::Request &>(req).remote_addr = req.get_header_value("X-Real-IP");
                                        }

                                        if (!config.listen.auth.path.empty()
                                        && !config.listen.auth.username.empty()
                                        && !config.listen.auth.password.empty()
                                        && fplus::is_prefix_of(config.listen.auth.path, req.path))
                                        {
                                           // verify username && password
                                           if (req.has_header("Authorization"))
                                           {
                                               std::string auth = req.get_header_value("Authorization");
                                               if (auth.find("Basic ") == 0)
                                               {
                                                   auth = auth.substr(6);
                                                   auth = brynet::base::crypto::base64_decode(auth);
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
                                        return httplib::Server::HandlerResponse::Unhandled; });

    for (const auto &hook : config.hooks)
    {
        std::string name = hook.name;
        std::string method = fplus::to_upper_case (hook.method);
        std::string path = fmt::format("{}{}", config.listen.prefix, hook.path);
        std::string command = hook.command;
        std::string content_type = hook.result.type;
        std::string content = fmt::format("{}", fmt::join(hook.result.content, "\n"));
        int command_timeout = hook.command_timeout;

        spdlog::info("Bind `{}` {} {} hook, with command `{}`", name, method, path, command);

        auto handler = [=, &server](const httplib::Request &req, httplib::Response &res)
        {
            spdlog::info("Trigger hook `{}`", name);

            kainjow::mustache::data context;
            kainjow::mustache::data request;
            kainjow::mustache::data response;
            kainjow::mustache::data headers;

            headers = kainjow::mustache::lambda{[&req](const std::string &name)
                                                {
                                                    return req.get_header_value(name.c_str());
                                                }};

            request.set("method", req.method);
            request.set("path", req.path);
            request.set("body", req.body);

            request.set("remote_addr", req.remote_addr);
            request.set("remote_port", std::to_string(req.remote_port));
            request.set("header", headers);

            response.set("content_length", std::to_string(req.body.size()));
            response.set("content_type", content_type);

            context.set("name", name);
            context.set("command", command);
            context.set("app", VersionHelper::getInstance().AppName);
            context.set("version", VersionHelper::getInstance().Version);
            context.set("hash", VersionHelper::getInstance().CommitHash);
            context.set("request", request);
            context.set("response", response);

            auto rendered_command = kainjow::mustache::mustache{command}.render(context);
            auto command_output_future = PlatformHelper::getInstance().executeCommandAsync(rendered_command);
            context.set("rendered_command", rendered_command);
            context.set("command_output", kainjow::mustache::lambda_t{[&command_output_future, command_timeout](const std::string &)
                                                                      {
                                                                          if (command_timeout > 0)
                                                                          {
                                                                              spdlog::info("Waiting for command output... (timeout: {}ms)", command_timeout);
                                                                              if (command_output_future.wait_for(std::chrono::milliseconds(command_timeout)) == std::future_status::ready)
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
                                                                          }
                                                                      }});

            kainjow::mustache::mustache content_tmpl{content};
            auto result = content_tmpl.render(context);
            if (!content_tmpl.is_valid())
            {
                spdlog::error("Render content failed: {}", content_tmpl.error_message());
                res.status = 500;
                res.set_content("Render content failed", "text/plain");
                return;
            }
            res.set_content(result, content_type.c_str());
            if (result.size() > 1024)
            {
                spdlog::debug("Render: \n\r{}...\n", result.substr(0, 1024));
            }
            else
            {
                spdlog::debug("Render: \n\r{}\n", result);
            }
        };
        if (method == "GET")
        {
            server.Get(path.c_str(), handler);
        }
        else if (method == "POST")
        {
            server.Post(path.c_str(), handler);
        }
        else if (method == "Delete")
        {
            server.Delete(path.c_str(), handler);
        }
        else if (method == "Put")
        {
            server.Put(path.c_str(), handler);
        }
        else if (method == "Options")
        {
            server.Options(path.c_str(), handler);
        }
        else if (method == "Patch")
        {
            server.Patch(path.c_str(), handler);
        }
        else
        {
            spdlog::error("Illegal method: {}", method);
        }
    }

    spdlog::info("Server listen {}:{}.", config.listen.host, config.listen.port);

    server.listen_after_bind();
    configurator.exitMonitorLoop();
    return 0;
}
