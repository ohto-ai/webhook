#include <catch2/catch.hpp>
#include <server/cors_middleware.h>

using ohtoai::CorsMiddleware;
using ohtoai::CorsSettings;

TEST_CASE("CorsMiddleware: disabled does nothing", "[cors]")
{
    CorsSettings settings;
    settings.enabled = false;
    CorsMiddleware middleware(settings);

    httplib::Request req;
    httplib::Response res;
    req.method = "GET";

    auto result = middleware.handle(req, res);
    REQUIRE(result == httplib::Server::HandlerResponse::Unhandled);
}

TEST_CASE("CorsMiddleware: enabled injects headers", "[cors]")
{
    CorsSettings settings;
    settings.enabled = true;
    settings.allowed_origins = "https://example.com";
    CorsMiddleware middleware(settings);

    httplib::Request req;
    httplib::Response res;
    req.method = "GET";

    auto result = middleware.handle(req, res);
    REQUIRE(result == httplib::Server::HandlerResponse::Unhandled);
    REQUIRE(res.has_header("Access-Control-Allow-Origin"));
    REQUIRE(res.get_header_value("Access-Control-Allow-Origin") == "https://example.com");
}

TEST_CASE("CorsMiddleware: handles OPTIONS preflight", "[cors]")
{
    CorsSettings settings;
    settings.enabled = true;
    CorsMiddleware middleware(settings);

    httplib::Request req;
    httplib::Response res;
    req.method = "OPTIONS";

    auto result = middleware.handle(req, res);
    REQUIRE(result == httplib::Server::HandlerResponse::Handled);
    REQUIRE(res.status == 204);
}

TEST_CASE("CorsMiddleware: allow credentials header", "[cors]")
{
    CorsSettings settings;
    settings.enabled = true;
    settings.allow_credentials = true;
    CorsMiddleware middleware(settings);

    httplib::Request req;
    httplib::Response res;
    req.method = "GET";

    middleware.handle(req, res);
    REQUIRE(res.has_header("Access-Control-Allow-Credentials"));
    REQUIRE(res.get_header_value("Access-Control-Allow-Credentials") == "true");
}

TEST_CASE("CorsMiddleware: max age header", "[cors]")
{
    CorsSettings settings;
    settings.enabled = true;
    settings.max_age_seconds = 3600;
    CorsMiddleware middleware(settings);

    httplib::Request req;
    httplib::Response res;
    req.method = "GET";

    middleware.handle(req, res);
    REQUIRE(res.has_header("Access-Control-Max-Age"));
    REQUIRE(res.get_header_value("Access-Control-Max-Age") == "3600");
}
