#pragma once

#include <nlohmann/json.hpp>

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

struct SecurityConfig
{
    std::string webhook_secret = "";
    std::vector<std::string> trusted_proxies = {"127.0.0.1", "::1"};
    bool require_https = false;
    size_t max_body_size = 1048576;
    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(SecurityConfig, webhook_secret, trusted_proxies, require_https, max_body_size)
};

struct RateLimitConfig
{
    bool enabled = false;
    size_t max_requests = 100;
    int window_seconds = 60;
    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(RateLimitConfig, enabled, max_requests, window_seconds)
};

struct CorsConfig
{
    bool enabled = false;
    std::string allowed_origins = "*";
    std::string allowed_methods = "GET, POST, PUT, DELETE, PATCH, OPTIONS";
    std::string allowed_headers = "Content-Type, Authorization";
    bool allow_credentials = false;
    int max_age_seconds = 86400;
    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(CorsConfig, enabled, allowed_origins, allowed_methods, allowed_headers, allow_credentials, max_age_seconds)
};

struct TlsConfig
{
    bool enabled = false;
    std::string cert_file = "";
    std::string key_file = "";
    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(TlsConfig, enabled, cert_file, key_file)
};

struct WebhookConfigModal
{
    std::vector<Hook> hooks;
    Listen listen;
    Log log;
    SecurityConfig security;
    RateLimitConfig rate_limit;
    CorsConfig cors;
    TlsConfig tls;
    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(WebhookConfigModal, hooks, listen, log, security, rate_limit, cors, tls)

    static WebhookConfigModal generate()
    {
        WebhookConfigModal config;
        Hook demoHook {
            "echo -n \"Hello\"",
            "GET",
            "hi",
            "/hi",
            {
                "text/html",
                {
                    "<html>",
                    "<head>",
                    "    <link rel=\"shortcut icon\" href=\"favicon.ico\" type=\"image/svg+xml\">",
                    "    <link rel=\"icon\" href=\"favicon.ico\" type=\"image/svg+xml\">",
                    "</head>",
                    "<body>",
                    "   <h1>{{context.app}} {{context.version}} [{{context.commit_hash}}]</h1>",
                    "   <p>Method: {{request.method}}</p>",
                    "   <p>Path: {{request.path}}</p>",
                    "   <p>User-Agent: {{request.header.user-agent}}</p>",
                    "   <p>Client: {{request.remote_addr}}</p>",
                    "   <p>{{command_output}}</p>",
                    "</body>",
                    "</html>"
                },
            },
            8000,
        };

        config.hooks.push_back(demoHook);
        return config;
    }
};
