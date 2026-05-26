#include <catch2/catch.hpp>
#include <config/config_model.hpp>
#include <nlohmann/json.hpp>

TEST_CASE("ConfigModel default values", "[config]")
{
    WebhookConfigModal config;

    REQUIRE(config.listen.host == "localhost");
    REQUIRE(config.listen.port == 8080);
    REQUIRE(config.listen.prefix == "/api");
    REQUIRE(config.listen.auth.username.empty());
    REQUIRE(config.listen.auth.password.empty());
    REQUIRE(config.log.console_level == "info");
    REQUIRE(config.log.file_level == "info");
    REQUIRE(config.hooks.empty());
    REQUIRE(config.security.max_body_size == 1048576);
    REQUIRE_FALSE(config.rate_limit.enabled);
    REQUIRE_FALSE(config.cors.enabled);
    REQUIRE_FALSE(config.tls.enabled);
}

TEST_CASE("ConfigModel JSON deserialization", "[config]")
{
    std::string json = R"({
        "hooks": [
            {
                "command": "echo test",
                "method": "GET",
                "name": "test-hook",
                "path": "/test"
            }
        ],
        "listen": {
            "host": "0.0.0.0",
            "port": 9000,
            "prefix": "/api"
        },
        "log": {
            "console_level": "debug"
        }
    })";

    nlohmann::json j = nlohmann::json::parse(json);
    auto config = j.get<WebhookConfigModal>();

    REQUIRE(config.listen.host == "0.0.0.0");
    REQUIRE(config.listen.port == 9000);
    REQUIRE(config.hooks.size() == 1);
    REQUIRE(config.hooks[0].name == "test-hook");

    // Backward compat: missing new sections get defaults
    REQUIRE(config.security.max_body_size == 1048576);
    REQUIRE_FALSE(config.rate_limit.enabled);
}

TEST_CASE("ConfigModel generate creates default config", "[config]")
{
    auto config = WebhookConfigModal::generate();

    REQUIRE(config.hooks.size() == 1);
    REQUIRE(config.hooks[0].name == "hi");
    REQUIRE(config.hooks[0].method == "GET");
    REQUIRE(config.hooks[0].path == "/hi");
    REQUIRE(config.hooks[0].command_timeout == 8000);
    REQUIRE(config.hooks[0].result.type == "text/html");
    REQUIRE_FALSE(config.hooks[0].result.content.empty());
}

TEST_CASE("ConfigModel with rate limit", "[config]")
{
    std::string json = R"({
        "hooks": [],
        "listen": {"host": "localhost", "port": 8080},
        "rate_limit": {"enabled": true, "max_requests": 50, "window_seconds": 30}
    })";

    nlohmann::json j = nlohmann::json::parse(json);
    auto config = j.get<WebhookConfigModal>();

    REQUIRE(config.rate_limit.enabled);
    REQUIRE(config.rate_limit.max_requests == 50);
    REQUIRE(config.rate_limit.window_seconds == 30);
}

TEST_CASE("ConfigModel with CORS settings", "[config]")
{
    std::string json = R"({
        "hooks": [],
        "listen": {"host": "localhost", "port": 8080},
        "cors": {"enabled": true, "allowed_origins": "https://example.com"}
    })";

    nlohmann::json j = nlohmann::json::parse(json);
    auto config = j.get<WebhookConfigModal>();

    REQUIRE(config.cors.enabled);
    REQUIRE(config.cors.allowed_origins == "https://example.com");
}

TEST_CASE("ConfigModel with TLS settings", "[config]")
{
    std::string json = R"({
        "hooks": [],
        "listen": {"host": "localhost", "port": 8080},
        "tls": {"enabled": true, "cert_file": "/path/cert.pem", "key_file": "/path/key.pem"}
    })";

    nlohmann::json j = nlohmann::json::parse(json);
    auto config = j.get<WebhookConfigModal>();

    REQUIRE(config.tls.enabled);
    REQUIRE(config.tls.cert_file == "/path/cert.pem");
    REQUIRE(config.tls.key_file == "/path/key.pem");
}
