#pragma once

#include <nlohmann/json.hpp>
#include <ghc/fs_std_fwd.hpp>

struct BasicAuth
{
    std::string username = "";
    std::string password = "";
    std::string path = "/";
    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(BasicAuth, username, password, path)
};

struct Result
{
    std::string type = "text/plain";
    std::vector<std::string> content = {};
    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(Result, type, content)
};

struct Hook
{
    std::string command = "";
    std::string method = "GET";
    std::string name = "";
    std::string path = "";
    Result result;
    int command_timeout = 8000;
    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(Hook, command, method, name, path, result, command_timeout)
};

struct Listen
{
    BasicAuth auth;
    std::string host = "localhost";
    int port = 8080;
    std::string prefix = "/api";
    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(Listen, auth, host, port, prefix)
};

struct Log
{
    std::string console_level = "info";
    std::string file_level = "info";
    std::string file_path = "webhook.log";
    std::string global_level = "info";
    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(Log, console_level, file_level, file_path, global_level)
};

struct WebhookConfigModal
{
    std::vector<Hook> hooks;
    Listen listen;
    Log log;
    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(WebhookConfigModal, hooks, listen, log)

    static WebhookConfigModal load(const std::string &configPath)
    {
        fs::ifstream ifs(configPath);
        if (!ifs.is_open())
        {
            throw std::runtime_error(std::string("Failed to open config file: ") + configPath);
        }

        WebhookConfigModal config;
        try
        {
            nlohmann::json configJ;
            ifs >> configJ;
            configJ.get_to(config);
        }
        catch (std::exception e)
        {
            throw std::runtime_error(std::string("Failed to parse config file: ") + e.what());
        }

        ifs.close();
        return config;
    }

    static void save(const std::string &configPath, const WebhookConfigModal &config)
    {
        nlohmann::json configJ = config;
        fs::ofstream ofs(configPath);
        ofs << configJ.dump(4);
        ofs.close();
    }

    static void generate(const std::string &configPath)
    {
        WebhookConfigModal config;
        Hook demoHook {
            "echo -n \"Hello\"", // command
            "GET",               // method
            "hi",                // name
            "/hi",               // path
            {
                "text/html", // type
                {
                    "<h1>{{&app}} {{&version}}{{#hash}}({{.}}){{/hash}}</h1>",
                    "<p>Method: {{&request.method}}</p>",
                    "<p>Path: {{&request.path}}</p>",
                    "<p>User-Agent: {{#request.header}}user-agent{{/request.header}}</p>",
                    "<p>Client: {{request.remote_addr}}</p>",
                    "<p>{{&command_output}}</p>"}, // content
            },                                     // result
            8000,                                  // command_timeout
        };

        config.hooks.push_back(demoHook);
        save(configPath, config);
    }
};
