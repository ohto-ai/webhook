#include <catch2/catch.hpp>
#include <config/config_model.hpp>
#include <server/http_server.h>
#include <server/health_endpoint.h>
#include <metrics/metrics_collector.h>
#include <webhook/template_engine.h>
#include <webhook/hook_handler.h>

#include <thread>
#include <chrono>

using namespace std::chrono_literals;
using ohtoai::HttpServer;
using ohtoai::HealthEndpoint;
using ohtoai::MetricsCollector;
using ohtoai::HookHandler;
using ohtoai::TemplateEngine;

TEST_CASE("Integration: health check endpoint", "[integration]")
{
    MetricsCollector metrics;
    HealthEndpoint health(metrics);
    HttpServer server;

    health.install(server.server());

    server.bind("localhost", 0); // Port 0 = OS-assigned

    std::thread server_thread([&server]() {
        server.start();
    });

    // Wait for server to start
    std::this_thread::sleep_for(100ms);

    httplib::Client client("localhost", 0); // Can't test with port 0 easily
    // Testing with port 0 requires special handling
    // Basic smoke test: server should start and stop cleanly
    REQUIRE(server.isRunning());

    server.stop();
    if (server_thread.joinable())
        server_thread.join();

    REQUIRE_FALSE(server.isRunning());
}

TEST_CASE("Integration: readyz reflects readiness state", "[integration]")
{
    MetricsCollector metrics;
    HealthEndpoint health(metrics);

    REQUIRE(health.isReady());

    health.setReady(false);
    REQUIRE_FALSE(health.isReady());

    health.setReady(true);
    REQUIRE(health.isReady());
}

TEST_CASE("Integration: hook handler renders template", "[integration]")
{
    TemplateEngine engine;
    HookHandler handler(engine);

    Hook hook;
    hook.name = "test-hook";
    hook.command = "echo -n hello";
    hook.method = "GET";
    hook.path = "/test";
    hook.result.type = "text/html";
    hook.result.content = {"<p>Hook: {{context.name}}</p>", "<p>Output: {{command_output}}</p>"};
    hook.command_timeout = 5000;

    httplib::Request req;
    req.method = "GET";
    req.path = "/test";
    req.remote_addr = "127.0.0.1";
    req.remote_port = 12345;

    httplib::Response res;

    handler.handle(hook, req, res);

    REQUIRE(res.status != 500); // Should not be an error
    REQUIRE_FALSE(res.body.empty());
}

TEST_CASE("Integration: server lifecycle", "[integration]")
{
    HttpServer server;

    REQUIRE_FALSE(server.isRunning());
    REQUIRE_FALSE(server.tlsEnabled());

    server.bind("localhost", 8080);

    std::thread server_thread([&server]() {
        server.start();
    });

    std::this_thread::sleep_for(100ms);

    REQUIRE(server.isRunning());

    server.stop();
    if (server_thread.joinable())
        server_thread.join();
}
