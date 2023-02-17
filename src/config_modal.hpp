#pragma once
#include <nlohmann/json.hpp>
#include <spdlog/fmt/fmt.h>

struct BasicAuth
{
    std::string username = "";
    std::string password = "";
    std::string path = "/";
    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(BasicAuth, username, password, path)
};

struct MultiLine
{
    std::string str = "";
    friend void to_json(nlohmann::json &nlohmann_json_j, const MultiLine &nlohmann_json_t)
    {
        NLOHMANN_JSON_TO(str);
    }
    friend void from_json(const nlohmann::json &nlohmann_json_j, MultiLine &nlohmann_json_t)
    {
        if (nlohmann_json_j.is_array())
        {
            nlohmann_json_t.str = fmt::format("{}", fmt::join(nlohmann_json_j.get<std::vector<std::string>>(), "\n"));
        }
        else
        {
            nlohmann_json_t.str = nlohmann_json_j.value("str", "");
        }
    }
};

struct Result
{
    std::string type = "text/plain";
    MultiLine content = {};
    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(Result, type, content)
};

struct Hook
{
    std::string command = "";
    std::string method = "GET";
    std::string name = "";
    std::string path = "";
    Result result;
    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(Hook, command, method, name, path, result)
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
        std::ifstream ifs(configPath);
        if (!ifs.is_open())
        {
            throw std::runtime_error(fmt::format("Failed to open config file: {}", configPath));
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
            throw std::runtime_error(fmt::format("Failed to parse config file: {}", e.what()));
        }

        ifs.close();
        return config;
    }

    static void save(const std::string &configPath, const WebhookConfigModal &config)
    {
        nlohmann::json configJ = config;
        std::ofstream ofs(configPath);
        ofs << configJ.dump(4);
        ofs.close();
    }

    static void generate(const std::string &configPath)
    {
        WebhookConfigModal config;
        Hook demoHook = {
            .command = "echo -n \"Hello\"",
            .method = "GET",
            .name = "hi",
            .path = "/hi",
            .result = {
                .type = "text/html",
                .content = {
                    .str = R"(<h1>{{#response}}{{&command_output}}{{/response}} {{&app}} {{&version}}</h1>
{{#request}}
<p>Method: {{&method}}</p>
<p>Path: {{&path}}</p>
<p>User-Agent: {{&user_agent}}</p>
<p>Client: {{&remote_addr}}:{{&remote_port}}</p>
{{/request}})"
                },
            },
        };
        config.hooks.push_back(demoHook);
        save(configPath, config);
    }
};
