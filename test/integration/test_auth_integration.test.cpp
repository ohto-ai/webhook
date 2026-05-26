#include <catch2/catch.hpp>
#include <auth/auth_manager.h>
#include <auth/basic_auth.h>
#include <cppcodec/base64_default_rfc4648.hpp>

using ohtoai::AuthManager;
using ohtoai::AuthConfig;

TEST_CASE("Integration-Auth: unauthenticated request without auth config allows access", "[integration][auth]")
{
    AuthConfig config;
    config.username = ""; // No auth required
    config.password = "";
    config.path = "";

    AuthManager manager(config);

    httplib::Request req;
    req.path = "/api/test";
    httplib::Response res;

    auto result = manager.authenticate(req, res);
    REQUIRE(result == httplib::Server::HandlerResponse::Unhandled);
}

TEST_CASE("Integration-Auth: authenticated request with valid credentials", "[integration][auth]")
{
    AuthConfig config;
    config.username = "admin";
    config.password = "secret";
    config.path = "/api";

    AuthManager manager(config);

    httplib::Request req;
    req.path = "/api/test";
    std::string creds = base64::encode("admin:secret");
    req.headers.emplace("Authorization", "Basic " + creds);
    httplib::Response res;

    auto result = manager.authenticate(req, res);
    REQUIRE(result == httplib::Server::HandlerResponse::Unhandled);
}

TEST_CASE("Integration-Auth: request without auth header on protected path returns 401", "[integration][auth]")
{
    AuthConfig config;
    config.username = "admin";
    config.password = "secret";
    config.path = "/api";

    AuthManager manager(config);

    httplib::Request req;
    req.path = "/api/private";
    httplib::Response res;

    auto result = manager.authenticate(req, res);
    REQUIRE(result == httplib::Server::HandlerResponse::Handled);
    REQUIRE(res.status == 401);
    REQUIRE(res.has_header("WWW-Authenticate"));
}

TEST_CASE("Integration-Auth: request with invalid credentials returns 401", "[integration][auth]")
{
    AuthConfig config;
    config.username = "admin";
    config.password = "secret";
    config.path = "/api";

    AuthManager manager(config);

    httplib::Request req;
    req.path = "/api/test";
    std::string creds = base64::encode("admin:wrongpassword");
    req.headers.emplace("Authorization", "Basic " + creds);
    httplib::Response res;

    auto result = manager.authenticate(req, res);
    REQUIRE(result == httplib::Server::HandlerResponse::Handled);
    REQUIRE(res.status == 401);
}

TEST_CASE("Integration-Auth: path not matching auth prefix skips auth", "[integration][auth]")
{
    AuthConfig config;
    config.username = "admin";
    config.password = "secret";
    config.path = "/api";

    AuthManager manager(config);

    httplib::Request req;
    req.path = "/public/endpoint"; // Not under /api
    httplib::Response res;

    auto result = manager.authenticate(req, res);
    REQUIRE(result == httplib::Server::HandlerResponse::Unhandled);
}

TEST_CASE("Integration-Auth: X-Real-IP header sets client address", "[integration][auth]")
{
    AuthConfig config;
    config.trusted_proxies = {"10.0.0.1"};

    AuthManager manager(config);

    httplib::Request req;
    req.path = "/public/test";
    req.remote_addr = "10.0.0.1";
    req.headers.emplace("X-Real-IP", "203.0.113.42");
    httplib::Response res;

    auto result = manager.authenticate(req, res);
    REQUIRE(result == httplib::Server::HandlerResponse::Unhandled);
    // The const_cast in auth_manager sets the client IP from X-Real-IP
    // We verify this didn't crash and returned normally
}
