#include <cpp-httplib/httplib.h>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/fmt/bundled/color.h>
#include <spdlog/fmt/fmt.h>
#include <mustache/mustache.hpp>
#include <argparse/argparse.hpp>
#include <fstream>
#include <algorithm>
#include <iterator>
#include <brynet/base/crypto/Base64.hpp>
#include "ohtoai/string_tools.hpp"
#include "config_modal.hpp"
#include "util/platform.hpp"
#include "util/compiler.hpp"
#include "version.h"

int main(int argc, char **argv)
{
    constexpr auto configPath{"hook.json"};
    argparse::ArgumentParser parser(APPNAME, VERSION_STRING);

    try
    {
        parser.parse_args(argc, argv);
    }
    catch (const std::runtime_error &err)
    {
        fmt::print(stderr, "{}\n", err.what());
        fmt::print(stderr, "{}\n", parser.help().str());
        std::exit(1);
    }

    fmt::print(fg(fmt::color::gold), "{}\n", fmt::join(AsciiBanner, "\n"));
    fmt::print("{} start.\n", CompilerHelper::getInstance().AppName);
    fmt::print("Version {}({})\ton {}\n", VERSION_STRING, CompilerHelper::getInstance().CodeVersion, CompilerHelper::getInstance().CodeDate);
    fmt::print("Build on {} {} {}\n", CompilerHelper::getInstance().BuildMachineInfo, CompilerHelper::getInstance().BuildDate, CompilerHelper::getInstance().BuildTime);
    fmt::print("Code hosted at {}\n", CompilerHelper::getInstance().CodeServerPath);
    fmt::print("Load config {}\n", configPath);

    WebhookConfigModal config;
    std::ifstream ifs(configPath);
    if (!ifs)
    {
        Hook demoHook = {
            .command = "echo -n \"Hello\"",
            .method = "GET",
            .name = "hi",
            .path = "/hi",
            .result = {
                .type = "text/html",
                .content = "<h1>{{&command_output}} {{&app}}</h1>",
            },
        };
        config.hooks.push_back(demoHook);
        nlohmann::json configJ = config;
        std::ofstream ofs(configPath);
        ofs << configJ.dump(4);
        ofs.close();
        spdlog::info("{} generated.", configPath);
        return 0;
    }

    try
    {
        nlohmann::json configJ;
        ifs >> configJ;
        configJ.get_to(config);
    }
    catch (std::exception e)
    {
        spdlog::error("{}", e.what());
        return -1;
    }

    ifs.close();

    // Init log
    try
    {
        auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        console_sink->set_level(spdlog::level::from_str(config.log.console_level));

        auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(config.log.file_path, true);
        file_sink->set_level(spdlog::level::from_str(config.log.file_level));

        spdlog::set_default_logger(std::make_shared<spdlog::logger>("webhook", spdlog::sinks_init_list({console_sink, file_sink})));
        spdlog::set_level(spdlog::level::from_str(config.log.global_level));    
        spdlog::flush_every(std::chrono::seconds(5));
    }
    catch (const spdlog::spdlog_ex &ex)
    {
        fmt::print(stderr, "Log initialization failed: {}\n", ex.what());
    }

    spdlog::info("Config loaded.");

    httplib::Server server;
    server.bind_to_port(config.listen.host.c_str(), config.listen.port);

    server.set_logger([](const httplib::Request &req, const httplib::Response &res)
                      { spdlog::info("Response {} {}", req.method, req.path);
                      spdlog::info("Send {} bytes", res.body.size()); });

    if (!config.listen.auth.path.empty() && !config.listen.auth.username.empty() && !config.listen.auth.password.empty())
    {
        spdlog::info("Auth enabled: {} {} {}", config.listen.auth.path, config.listen.auth.username, config.listen.auth.password);
        server.set_pre_routing_handler([&config](const httplib::Request &req, httplib::Response &res)
                                       {
                                           if (!ohtoai::tool::string::start_with(req.path, config.listen.auth.path))
                                               return httplib::Server::HandlerResponse::Unhandled;
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
                                           return httplib::Server::HandlerResponse::Handled; });
    }

    for (const auto &hook : config.hooks)
    {
        std::string name = hook.name;
        std::string method = ohtoai::tool::string::to_upper(hook.method);
        std::string path = fmt::format("{}{}", config.listen.prefix, hook.path);
        std::string command = hook.command;
        std::string content_type = hook.result.type;
        std::string content = hook.result.content;

        spdlog::info("Bind `{}` {} {} hook, with command `{}`", name, method, path, command);

        auto handler = [=, &server](const httplib::Request &req, httplib::Response &res)
        {
            spdlog::info("Hook `{}`", name);

            std::string command_output{};
            if (!command.empty())
            {
                spdlog::info("Run `{}`", command);
                command_output = PlatformHelper::getInstance().executeCommand(command);
            }
            kainjow::mustache::data context;
            context.set("name", name);
            context.set("method", method);
            context.set("path", path);
            context.set("command", command);
            context.set("content_type", content_type);
            context.set("command_output", command_output);
            context.set("app", APPNAME);
            kainjow::mustache::mustache content_tmpl{content};

            auto result = content_tmpl.render(context);
            res.set_content(result, content_type.c_str());
            spdlog::info("Render: \n\r{}\n", result);
            fmt::print(fg(fmt::color::green), "\r{:=^80}\n", " Done ");
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

    fmt::print(fg(fmt::color::green), "\r{:=^80}\n", " Done ");
    spdlog::info("Server listen {}:{}.", config.listen.host, config.listen.port);

    return server.listen_after_bind();
}
