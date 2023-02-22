#include <cpp-httplib/httplib.h>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/fmt/bundled/color.h>
#include <spdlog/fmt/fmt.h>
#include <mustache/mustache.hpp>
#include <fstream>
#include <algorithm>
#include <iterator>
#include <brynet/base/crypto/Base64.hpp>
#include "ohtoai/string_tools.hpp"
#include "config_modal.hpp"
#include "util/platform.hpp"
#include "util/compiler.hpp"
#include "util/version.hpp"

int main(int argc, char **argv)
{
    constexpr auto configPath{"hook.json"};

    fmt::print(fg(fmt::color::gold), "{}\n", VersionHelper::getInstance().AsciiBanner);
    fmt::print(fg(fmt::color::green), "\r{:=^{}}\n", "=", PlatformHelper::getInstance().getTerminalWidth());
    fmt::print("Run {}.\n", CompilerHelper::getInstance().AppName);
    fmt::print("Version {}({}) on {}\n", VersionHelper::getInstance().Version, CompilerHelper::getInstance().CommitHash, CompilerHelper::getInstance().CommitDate);
    fmt::print("Build on {} {} {}\n", CompilerHelper::getInstance().BuildMachineInfo, CompilerHelper::getInstance().BuildDate, CompilerHelper::getInstance().BuildTime);
    fmt::print(fg(fmt::color::green), "\r{:=^{}}\n", "=", PlatformHelper::getInstance().getTerminalWidth());

    if (!access(configPath, R_OK))
    {
        WebhookConfigModal::generate(configPath);
        fmt::print("Config file not found, generate a new one.\n");
        return 0;
    }

    fmt::print("Load config {}\n", configPath);
    WebhookConfigModal config;
    try
    {
        config = WebhookConfigModal::load(configPath);
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
                                        && ohtoai::tool::string::start_with(req.path, config.listen.auth.path))
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
        std::string method = ohtoai::tool::string::to_upper(hook.method);
        std::string path = fmt::format("{}{}", config.listen.prefix, hook.path);
        std::string command = hook.command;
        std::string content_type = hook.result.type;
        std::string content = fmt::format("{}", fmt::join(hook.result.content, "\n"));
        int command_timeout = hook.command_timeout;

        spdlog::info("Bind `{}` {} {} hook, with command `{}`", name, method, path, command);

        auto handler = [=, &server](const httplib::Request &req, httplib::Response &res)
        {
            spdlog::info("Trigger hook `{}`", name);

            auto command_output_future = PlatformHelper::getInstance().executeCommandAsync(command);
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
            response.set("command_output", kainjow::mustache::lambda_t{[&command_output_future, command_timeout](const std::string &)
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

            context.set("name", name);
            context.set("command", command);
            context.set("app", CompilerHelper::getInstance().AppName);
            context.set("version", VersionHelper::getInstance().Version);
            context.set("hash", CompilerHelper::getInstance().CommitHash);
            context.set("request", request);
            context.set("response", response);
            context.set("file", kainjow::mustache::lambda_t{[](const std::string &file_path, const kainjow::mustache::renderer &render)
                                                            {
                                                                std::ifstream ifs(file_path);
                                                                if (!ifs.is_open())
                                                                {
                                                                    spdlog::error("File `{}` not found", file_path);
                                                                    return std::string{};
                                                                }
                                                                std::string content((std::istreambuf_iterator<char>(ifs)), (std::istreambuf_iterator<char>()));
                                                                return content;
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

    return server.listen_after_bind();
}
