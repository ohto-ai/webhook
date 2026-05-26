#include <catch2/catch.hpp>
#include <server/request_validator.h>

using ohtoai::RequestValidator;

TEST_CASE("RequestValidator: allows normal path", "[validator]")
{
    RequestValidator validator;
    httplib::Request req;
    httplib::Response res;
    req.path = "/api/hi";

    auto result = validator.validate(req, res);
    REQUIRE(result == httplib::Server::HandlerResponse::Unhandled);
}

TEST_CASE("RequestValidator: blocks path traversal with ..", "[validator]")
{
    RequestValidator validator;
    httplib::Request req;
    httplib::Response res;
    req.path = "/api/../../../etc/passwd";

    auto result = validator.validate(req, res);
    REQUIRE(result == httplib::Server::HandlerResponse::Handled);
    REQUIRE(res.status == 400);
}

TEST_CASE("RequestValidator: blocks path traversal with encoded ..", "[validator]")
{
    RequestValidator validator;
    httplib::Request req;
    httplib::Response res;
    req.path = "/api/%2e%2e/%2e%2e/etc/passwd";

    auto result = validator.validate(req, res);
    REQUIRE(result == httplib::Server::HandlerResponse::Handled);
}

TEST_CASE("RequestValidator: allows normal body size", "[validator]")
{
    RequestValidator validator(1024 * 1024); // 1 MB
    httplib::Request req;
    httplib::Response res;
    req.path = "/api/hook";
    req.body = std::string(1024, 'x');

    auto result = validator.validate(req, res);
    REQUIRE(result == httplib::Server::HandlerResponse::Unhandled);
}

TEST_CASE("RequestValidator: blocks oversized body", "[validator]")
{
    RequestValidator validator(100); // 100 bytes max
    httplib::Request req;
    httplib::Response res;
    req.path = "/api/hook";
    req.body = std::string(200, 'x');

    auto result = validator.validate(req, res);
    REQUIRE(result == httplib::Server::HandlerResponse::Handled);
    REQUIRE(res.status == 413);
}

TEST_CASE("RequestValidator: custom max body size", "[validator]")
{
    RequestValidator validator(500);
    httplib::Request req;
    httplib::Response res;
    req.path = "/api/hook";
    req.body = std::string(10, 'x');

    auto result = validator.validate(req, res);
    REQUIRE(result == httplib::Server::HandlerResponse::Unhandled);
}
