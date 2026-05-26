#include <catch2/catch.hpp>
#include <metrics/metrics_collector.h>

using namespace std::chrono_literals;
using ohtoai::MetricsCollector;
using ohtoai::RequestRecord;

TEST_CASE("MetricsCollector: initial state", "[metrics]")
{
    MetricsCollector metrics;

    REQUIRE(metrics.totalRequests() == 0);
    REQUIRE(metrics.totalErrors() == 0);
    REQUIRE(metrics.totalHooksExecuted() == 0);
    REQUIRE(metrics.averageLatencyMs() == 0.0);
}

TEST_CASE("MetricsCollector: record successful request", "[metrics]")
{
    MetricsCollector metrics;

    RequestRecord record;
    record.method = "GET";
    record.path = "/api/hello";
    record.status = 200;
    record.duration = 5000us; // 5ms

    metrics.recordRequest(record);

    REQUIRE(metrics.totalRequests() == 1);
    REQUIRE(metrics.totalErrors() == 0);
    REQUIRE(metrics.averageLatencyMs() == Approx(5.0));
}

TEST_CASE("MetricsCollector: record error request", "[metrics]")
{
    MetricsCollector metrics;

    RequestRecord record;
    record.method = "GET";
    record.path = "/api/bad";
    record.status = 500;
    record.duration = 10000us;

    metrics.recordRequest(record);

    REQUIRE(metrics.totalRequests() == 1);
    REQUIRE(metrics.totalErrors() == 1);
}

TEST_CASE("MetricsCollector: record hook execution", "[metrics]")
{
    MetricsCollector metrics;

    metrics.recordHookExecution("my-hook", 5000us, true);
    REQUIRE(metrics.totalHooksExecuted() == 1);

    metrics.recordHookExecution("my-hook", 3000us, false);
    REQUIRE(metrics.totalHooksExecuted() == 2);
    REQUIRE(metrics.totalErrors() == 1);

    metrics.recordHookExecution("other-hook", 10000us, true);
    REQUIRE(metrics.totalHooksExecuted() == 3);
}

TEST_CASE("MetricsCollector: snapshot contains all fields", "[metrics]")
{
    MetricsCollector metrics;

    RequestRecord record;
    record.method = "POST";
    record.path = "/api/test";
    record.status = 201;
    record.duration = 2000us;
    metrics.recordRequest(record);

    auto json = metrics.snapshot();

    REQUIRE(json.contains("total_requests"));
    REQUIRE(json.contains("total_errors"));
    REQUIRE(json.contains("total_hooks_executed"));
    REQUIRE(json.contains("average_latency_ms"));
    REQUIRE(json["total_requests"] == 1);
}

TEST_CASE("MetricsCollector: thread safety stress test", "[metrics]")
{
    MetricsCollector metrics;

    auto worker = [&metrics]() {
        for (int i = 0; i < 1000; ++i) {
            RequestRecord record;
            record.method = "GET";
            record.path = "/api/test";
            record.status = i % 100 == 0 ? 500 : 200;
            record.duration = 1000us;
            metrics.recordRequest(record);
        }
    };

    std::thread t1(worker);
    std::thread t2(worker);
    std::thread t3(worker);

    t1.join();
    t2.join();
    t3.join();

    REQUIRE(metrics.totalRequests() == 3000);
}
