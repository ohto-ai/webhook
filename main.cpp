#include <cpp-httplib/httplib.h>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <fstream>
#include <algorithm>
#include <iterator>
#include <brynet/base/crypto/Base64.hpp>
#include "ohtoai/string_tools.hpp"

std::string executeCommand(std::string cmd)
{
#   ifdef _WIN32
#   define popen _popen
#   define pclose _pclose
#   endif
    auto f = popen(cmd.c_str(), "r");
    std::string display{};
    if (f != nullptr)
    {
        char buf_ps[1024];
        while (fgets(buf_ps, sizeof(buf_ps), f) != nullptr)
            display += buf_ps;
        pclose(f);
    }
    else
    {
        spdlog::error("Cannot open pip `{}`", cmd);
    }
    return display;

#   ifdef _WIN32
#   undef popen
#   undef pclose
#   endif
}

struct UserLogin {
    std::string username;
    std::string password;
};

int main()
{
    constexpr auto configPath{ "hook.json" };

    spdlog::info("Application start.");
    spdlog::info("Load config {}", configPath);
    std::ifstream ifs(configPath);
    if (!ifs) {
        spdlog::error("Failed to open hook.json");
        return -1;
    }

    nlohmann::json configJ;
    try {
        ifs >> configJ;
    }
    catch (nlohmann::detail::exception e)
    {
        spdlog::error("{}", e.what());
        return -1;
    }

    ifs.close();

    using namespace nlohmann::literals;
    auto log_console_level_ptr = "/log/console_level"_json_pointer;
    auto log_file_level_ptr = "/log/file_level"_json_pointer;
    auto log_file_path_ptr = "/log/file_path"_json_pointer;
    auto log_global_level_ptr = "/log/global_level"_json_pointer;

    std::string log_console_level   = configJ.contains(log_console_level_ptr)   ? configJ[log_console_level_ptr]: "info";
    std::string log_file_level      = configJ.contains(log_file_level_ptr)      ? configJ[log_file_level_ptr]   : "info";
    std::string log_file_path       = configJ.contains(log_file_path_ptr)       ? configJ[log_file_path_ptr]    : "hook.log";
    std::string log_global_level    = configJ.contains(log_global_level_ptr)    ? configJ[log_global_level_ptr] : "info";

    try
    {
        auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        console_sink->set_level(spdlog::level::from_str(log_console_level));

        auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(log_file_path, true);
        file_sink->set_level(spdlog::level::from_str(log_file_level));

        spdlog::set_default_logger(std::make_shared<spdlog::logger>("webhook", spdlog::sinks_init_list({ console_sink, file_sink })));
        spdlog::set_level(spdlog::level::from_str(log_global_level));
    }
    catch (const spdlog::spdlog_ex& ex)
    {
        std::cout << "Log initialization failed: " << ex.what() << std::endl;
    }

    spdlog::info("Config loaded.");

    // auth
    UserLogin globalUserLogin {};
    bool isAuthEnabled { false };
    if (configJ.contains("auth")
        && configJ["auth"].contains("username")
        && configJ["auth"].contains("password"))
    {
        globalUserLogin.username = configJ["auth"]["username"];
        globalUserLogin.password = configJ["auth"]["password"];
        isAuthEnabled = configJ["auth"].contains("enabled") ? configJ["auth"]["enabled"].get<bool>() : true;
    }

    if (!configJ.contains("listen") || !configJ.contains("hook"))
    {
        spdlog::error("{} and {} NOT FOUND in {}!", "listen", "hook", "hook.json");
        return -1;
    }

    std::string pathPrefix{};
    if (configJ["listen"].contains("prefix"))
        pathPrefix = configJ["listen"]["prefix"];

    httplib::Server server;
    server.bind_to_port(configJ["listen"]["host"].get<std::string>().c_str(), configJ["listen"]["port"]);

    server.set_pre_routing_handler([&server, &globalUserLogin, isAuthEnabled, pathPrefix](const httplib::Request& req, httplib::Response& res)
        {
            if (!ohtoai::tool::string::start_with(req.path, pathPrefix))
            {
                spdlog::warn("Refuse to path {}", req.path);
                return httplib::Server::HandlerResponse::Unhandled;
            }

            spdlog::info("Routing {} {} \nReceive {} bytes\n{}", req.method, req.path, req.body.size(), req.body);

            if (isAuthEnabled)
            {
                if (req.method == "GET")
                {
                    auto auth_verified = [&globalUserLogin](const auto&req){
                        if (!req.has_header("Authorization") || req.get_header_value("Authorization").empty())
                        {
                            return false;
                        }
                        auto basic_auth_base64 = ohtoai::tool::string::split(
                            ohtoai::tool::string::trimmed(req.get_header_value("Authorization")), " ").back();
                        auto auth = ohtoai::tool::string::split(brynet::base::crypto::base64_decode(basic_auth_base64), ":");

                        std::transform(auth.begin(), auth.end(), auth.begin(), ohtoai::tool::string::trimmed);
                        if (auth.size() != 2)
                        {
                            spdlog::error("Error base64: {}", basic_auth_base64);
                            return false;
                        }

                        return auth[0] == globalUserLogin.username && auth[1] == globalUserLogin.password;
                    }(req);

                    if (!auth_verified)
                    {
                        res.set_header("WWW-Authenticate", "Basic");
                        res.status = 401;
                        return httplib::Server::HandlerResponse::Handled;
                    }

                    spdlog::info("Auth {} pass", globalUserLogin.username);
                }
            }

            return httplib::Server::HandlerResponse::Unhandled;
        });

    for (auto hook : configJ["hook"])
    {
        std::string name = hook["name"];
        std::string method = hook["method"];
        std::string path = pathPrefix + hook["path"].get<std::string>();
        std::string command{};
        std::string result_from = hook["result"]["from"];
        std::string result_type = hook["result"]["type"];
        std::string result_value{};

        if (hook.contains("command"))
        {
            if (hook["command"].is_array())
            {
                for (const auto& cmd: hook["command"])
                {
                    command += cmd.get<std::string>() + " && ";
                }
            }
            else
            {
                command = hook["command"].get<std::string>() + " && ";
            }
            command += "echo done";
        }
        if (hook["result"].contains("value"))
        {
            result_value = hook["result"]["value"];
        }

        if (result_type.empty())
            result_type = "text/plain";

        ohtoai::tool::string::transform_to_upper(method);

        spdlog::info("Bind `{}` {} {} hook, with command `{}`", name, method, path, command);

        auto handler = [=, &server](const httplib::Request& req, httplib::Response& res)
        {
            spdlog::info("Hook `{}`", name);
            std::string command_result{};

            if (command.empty())
            {
                spdlog::info("No command given");
            }
            else
            {
                command_result = executeCommand(command);
                spdlog::info("Run command `{}`\n{}", command, command_result);
            }

            if (result_from == "command")
            {
                res.set_content(command_result, result_type.c_str());
                spdlog::info("Set content to `{}`", command_result);
            }
            else if (result_from == "constant")
            {
                res.set_content(result_value, result_type.c_str());
                spdlog::info("Set content to `{}`", result_value);
            }
            else
            {
                spdlog::error("Unknown result_from `{}`", result_from);
            }
            spdlog::info("\n==================================================");
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
    spdlog::info("\n==================================================");
    spdlog::info("Server listen {}:{}.", configJ["listen"]["host"].get<std::string>(), configJ["listen"]["port"].get<int>());

    return server.listen_after_bind();
}
