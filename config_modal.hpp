#pragma once
#include <nlohmann/json.hpp>

struct BasicAuth {
    std::string username = "";
    std::string password = "";
    std::string path = "/";
    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(BasicAuth, username, password, path)
};

struct Result {
    std::string type = "text/plain";
    std::string content = ""; // {{command_output}}  {{command_return}} {{command}}
    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(Result, type, content)
};

struct Hook {
    std::string command = "";
    std::string method = "GET";
    std::string name = "";
    std::string path = "";
    Result result;
    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(Hook, command, method, name, path, result)
};

struct Listen {
    BasicAuth auth;
    std::string host = "localhost";
    int port = 8080;
    std::string prefix = "/api";
    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(Listen, auth, host, port, prefix)
};

struct Log {
    std::string console_level = "info";
    std::string file_level = "info";
    std::string file_path = "webhook.log";
    std::string global_level = "info";
    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(Log, console_level, file_level, file_path, global_level)
};

struct WebhookConfigModal {
    std::vector<Hook> hooks;
    Listen listen;
    Log log;
    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(WebhookConfigModal, hooks, listen, log)
};
