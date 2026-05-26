#include <catch2/catch.hpp>
#include <webhook/template_engine.h>
#include <nlohmann/json.hpp>

using ohtoai::TemplateEngine;

TEST_CASE("TemplateEngine: basic rendering", "[template]")
{
    TemplateEngine engine;

    nlohmann::json basic;
    basic["name"] = "test-app";
    engine.setBasicData(basic);

    nlohmann::json data;
    data["name"] = "world";

    std::string result = engine.render("Hello, {{name}}!", data);
    REQUIRE(result.find("Hello, world!") != std::string::npos);
}

TEST_CASE("TemplateEngine: template caching", "[template]")
{
    TemplateEngine engine;

    nlohmann::json data;
    data["value"] = "test1";

    std::string result1 = engine.render("Cached: {{value}}", data);

    data["value"] = "test2";
    std::string result2 = engine.render("Cached: {{value}}", data);

    REQUIRE(result1.find("test1") != std::string::npos);
    REQUIRE(result2.find("test2") != std::string::npos);
}

TEST_CASE("TemplateEngine: clear cache invalidates", "[template]")
{
    TemplateEngine engine;

    nlohmann::json data;
    data["val"] = "before";

    engine.render("{{val}}", data);
    engine.clearCache();

    data["val"] = "after";
    std::string result = engine.render("{{val}}", data);
    REQUIRE(result.find("after") != std::string::npos);
}

TEST_CASE("TemplateEngine: HTML escaping", "[template]")
{
    TemplateEngine engine;

    nlohmann::json data;
    engine.addCommandOutput(data, "<script>alert('xss')</script>");

    REQUIRE(data.contains("/command/output_html"));
    std::string escaped = data["/command/output_html"];
    REQUIRE(escaped.find("<script>") == std::string::npos);
    REQUIRE(escaped.find("&lt;script&gt;") != std::string::npos);
}

TEST_CASE("TemplateEngine: create render data", "[template]")
{
    TemplateEngine engine;

    nlohmann::json basic;
    basic["/context/app"] = "test-app";
    engine.setBasicData(basic);

    httplib::Request req;
    req.method = "GET";
    req.path = "/api/test";
    req.remote_addr = "192.168.1.1";
    req.remote_port = 54321;
    req.body = "request body";
    req.headers.emplace("User-Agent", "TestAgent/1.0");

    auto data = engine.createRenderData("my-hook", "echo test", req);

    REQUIRE(data["/request/method"] == "GET");
    REQUIRE(data["/request/path"] == "/api/test");
    REQUIRE(data["/request/remote_addr"] == "192.168.1.1");
    REQUIRE(data["/request/body"] == "request body");
    REQUIRE(data["/context/name"] == "my-hook");
    REQUIRE(data["/context/command"] == "echo test");
}

TEST_CASE("TemplateEngine: failing render on invalid template", "[template]")
{
    TemplateEngine engine;

    nlohmann::json data;
    REQUIRE_THROWS(engine.render("{% invalid %}", data));
}
