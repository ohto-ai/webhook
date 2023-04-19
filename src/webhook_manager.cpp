#include "webhook_manager.h"
#include "util/platform.h"
#include "version.hpp"

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/fmt/bundled/color.h>
#include <spdlog/fmt/fmt.h>
#include <fplus/fplus.hpp>
#include <cppcodec/base64_default_rfc4648.hpp>

#include <algorithm>

ohtoai::ExitReason ohtoai::WebhookManager::exec()
{
    welcome(); // Print welcome message

    if (!serve_precondition()
        || !doLoadConfig()
        || !installLoggers()
        || !installHooks())
    {
        spdlog::error("Failed to start server.");
        return ohtoai::Terminate;
    }
    spdlog::info("Server started.");

    config_monitor_thread = std::make_unique<std::thread>([this]()
    {
        while (server.is_running())
        {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            if (last_config_modify_time != std::filesystem::last_write_time(arg_config_path))
            {
                spdlog::info("Config file changed, reload.");
                exit_reason = ohtoai::Reload;
                server.stop();
                return;
            }
        }
    });

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

    if(!server.listen_after_bind())
        exit_reason = ohtoai::Terminate;

    return exit_reason;
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
    if (!std::filesystem::exists(arg_config_path))
    {
        if(arg_generate_config_if_not_exist)
        {
            spdlog::warn("Config file not found, generate a new one.\n");
            nlohmann::json j = WebhookConfigModal::generate();
            std::ofstream ofs(arg_config_path);
            ofs << j.dump(4) << std::endl;
            ofs.close();
            return !arg_quit_after_config_generate;
        }
        else
        {
            return false;
        }
    }

    return true;
}

bool ohtoai::WebhookManager::doLoadConfig()
{
    spdlog::info("Load config {}", arg_config_path.string());
    try
    {
        last_config_modify_time = std::filesystem::last_write_time(arg_config_path);
        nlohmann::json j;
        std::ifstream ifs(arg_config_path);
        ifs >> j;
        ifs.close();
        config = j;
    }
    catch (std::exception e)
    {
        spdlog::error("{}", e.what());
        return false;
    }
    spdlog::info("Config loaded.");
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

    server.Get(fmt::format("{}/favicon.ico", config.listen.prefix).c_str(), [this](const httplib::Request &req, httplib::Response &res)
        {
            res.set_content(default_icon_data, default_icon_type.c_str());
        });

    for (const auto &hook : config.hooks)
    {
        auto path = fmt::format("{}{}", config.listen.prefix, hook.path);
        auto content = fmt::format("{}", fmt::join(hook.result.content, "\n"));
        spdlog::info("Bind `{}` {} {} hook, with command `{}`", hook.name, hook.method, path, hook.command);
        auto handler = [&hook, content, this](const httplib::Request &req, httplib::Response &res)
        {
            spdlog::info("Trigger hook `{}`", hook.name);

            inja::Environment env = inja_env;
            nlohmann::json data = basic_render_data;
            fillEnv(env, data, hook, req, res);

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
                spdlog::error("Render content failed: {}\n\nContent:\n{}\nData:\n{}", e.what(), content, data.dump(4));
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

void ohtoai::WebhookManager::fillEnv(inja::Environment& env, nlohmann::json & data, const Hook& hook, const httplib::Request &req, httplib::Response &res)
{
    data["/context/name"_json_pointer] = hook.name;
    data["/context/command"_json_pointer] = hook.command;
    data["/request/method"_json_pointer] = req.method;
    data["/request/path"_json_pointer] = req.path;
    data["/request/body"_json_pointer] = req.body;
    data["/request/remote_addr"_json_pointer] = req.remote_addr;
    data["/request/remote_port"_json_pointer] = req.remote_port;
    for (const auto &[key, value] : req.headers)
        data["/request/header"_json_pointer][fplus::to_lower_case(key)] = value;
    data["/request/content_length"_json_pointer] = req.body.size();
    auto rendered_command = env.render(hook.command, data);
    data["/context/rendered_command"_json_pointer] = rendered_command;

    auto command_output_future = PlatformHelper::getInstance().executeCommandAsync(rendered_command);
    env.add_callback("command_output", 0, [&hook, command_output_future](inja::Arguments &args) -> nlohmann::json
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
}

ohtoai::WebhookManager::WebhookManager(int argc, char **argv) {
    auto program_path = std::filesystem::path(PlatformHelper::getInstance().getHomeDirectory()) / ".ohtoai" / VersionHelper::getInstance().AppName;
    if (!std::filesystem::exists(program_path))
    {
        std::filesystem::create_directories(program_path);
    }
    std::filesystem::current_path(program_path);
    
    // todo: Handle argv

    basic_render_data["/context/app"_json_pointer] = VersionHelper::getInstance().AppName;
    basic_render_data["/context/version"_json_pointer] = VersionHelper::getInstance().Version;
    basic_render_data["/context/commit_hash"_json_pointer] = VersionHelper::getInstance().CommitHash;
    basic_render_data["/context/commit_date"_json_pointer] = VersionHelper::getInstance().CommitDate;
    basic_render_data["/context/build_date"_json_pointer] = VersionHelper::getInstance().BuildDate;
    basic_render_data["/context/build_time"_json_pointer] = VersionHelper::getInstance().BuildTime;
    basic_render_data["/context/platform"_json_pointer] = PlatformHelper::getInstance().getPlatform();

    inja_env.add_void_callback("info", [](inja::Arguments &args)
                              {
        std::vector<std::string> result(args.size());
        std::transform(args.begin(), args.end(), result.begin(),
                [](const nlohmann::json* j) { return j->get<std::string>(); });
        spdlog::info("{}", fmt::join(result, " ")); });
    inja_env.add_void_callback("warn", [](inja::Arguments &args)
                              {
        std::vector<std::string> result(args.size());
        std::transform(args.begin(), args.end(), result.begin(),
                [](const nlohmann::json* j) { return j->get<std::string>(); });
        spdlog::warn("{}", fmt::join(result, " ")); });

    inja_env.add_void_callback("error", [](inja::Arguments &args)
                              {
        std::vector<std::string> result(args.size());
        std::transform(args.begin(), args.end(), result.begin(),
                [](const nlohmann::json* j) { return j->get<std::string>(); });
        spdlog::error("{}", fmt::join(result, " ")); });

    default_headers.emplace("Server", fmt::format("{} {}", VersionHelper::getInstance().AppName, VersionHelper::getInstance().Version));
    server.set_default_headers(default_headers);

    default_icon_type = "image/svg+xml";
    default_icon_data = R"FAVICON(<svg class="icon" style="width: 1em;height: 1em;vertical-align: middle;fill: currentColor;overflow: hidden;" viewBox="0 0 1024 1024" version="1.1" xmlns="http://www.w3.org/2000/svg"><path d="M96.228571 697.828571z m121.942858-105.485714c-28.571429 28.571429-44.228571 66.4-44.228572 106.742857s15.771429 78.285714 44.228572 106.742857c28.571429 28.571429 66.4 44.228571 106.742857 44.228572 40.342857 0 78.285714-15.771429 106.742857-44.228572l67.885714-67.885714-213.485714-213.485714-67.885714 67.885714z m480.914285-418.4c-40.342857 0-78.285714 15.771429-106.742857 44.228572l-67.885714 67.885714 213.485714 213.485714 67.885714-67.885714c28.457143-28.571429 44.228571-66.4 44.228572-106.742857s-15.771429-78.285714-44.228572-106.742857c-28.571429-28.571429-66.4-44.228571-106.742857-44.228572z" fill="#D9D9D9" /><path d="M588.457143 551.657143a9.177143 9.177143 0 0 0-12.914286 0L499.428571 627.771429 396.228571 524.571429l76.228572-76.228572c3.542857-3.542857 3.542857-9.371429 0-12.914286L430.857143 393.828571a9.177143 9.177143 0 0 0-12.914286 0L341.714286 470.057143l-49.142857-49.142857a8.971429 8.971429 0 0 0-6.514286-2.628572c-2.285714 0-4.685714 0.914286-6.514286 2.628572L163.2 537.371429a227.942857 227.942857 0 0 0-66.971429 160.457142c-0.228571 45.142857 12.8 90.4 39.2 129.257143l-86.971428 86.971429a9.177143 9.177143 0 0 0 0 12.914286l48.457143 48.457142c1.828571 1.828571 4.114286 2.628571 6.514285 2.628572s4.685714-0.914286 6.514286-2.628572l86.971429-86.971428c38.514286 26.171429 83.314286 39.2 128.114285 39.2 58.514286 0 117.028571-22.285714 161.714286-66.971429l116.457143-116.457143c3.542857-3.542857 3.542857-9.371429 0-12.914285l-49.142857-49.142857 76.228571-76.228572c3.542857-3.542857 3.542857-9.371429 0-12.914286l-41.828571-41.371428zM431.657143 805.828571a150.08 150.08 0 0 1-106.742857 44.228572c-40.342857 0-78.171429-15.657143-106.742857-44.228572-28.457143-28.457143-44.228571-66.4-44.228572-106.742857s15.657143-78.171429 44.228572-106.742857l67.885714-67.885714 213.485714 213.485714-67.885714 67.885714z m544-708.914285l-48.457143-48.457143c-1.828571-1.828571-4.114286-2.628571-6.514286-2.628572s-4.685714 0.914286-6.514285 2.628572l-86.971429 86.971428a227.737143 227.737143 0 0 0-128.114286-39.2c-58.514286 0-117.028571 22.285714-161.714285 66.971429L420.914286 279.657143a9.177143 9.177143 0 0 0 0 12.914286L731.428571 603.085714c1.828571 1.828571 4.114286 2.628571 6.514286 2.628572 2.285714 0 4.685714-0.914286 6.514286-2.628572l116.457143-116.457143c78.742857-78.857143 88-200.8 27.771428-289.714285l86.971429-86.971429c3.542857-3.657143 3.542857-9.485714 0-13.028571zM805.828571 431.657143l-67.885714 67.885714-213.485714-213.485714 67.885714-67.885714c28.457143-28.457143 66.4-44.228571 106.742857-44.228572s78.171429 15.657143 106.742857 44.228572c28.457143 28.457143 44.228571 66.4 44.228572 106.742857s-15.771429 78.171429-44.228572 106.742857z"  /></svg>)FAVICON";
}

ohtoai::WebhookManager::~WebhookManager() {
    if (server.is_running())
        server.stop();
    if (config_monitor_thread && config_monitor_thread->joinable())
        config_monitor_thread->join();
}
