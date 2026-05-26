#include "metrics_collector.h"

namespace ohtoai {

void MetricsCollector::recordRequest(const RequestRecord& record)
{
    total_requests_.fetch_add(1);
    total_latency_us_.fetch_add(record.duration.count());

    if (record.status >= 400)
        total_errors_.fetch_add(1);
}

void MetricsCollector::recordHookExecution(const std::string& hook_name,
                                            std::chrono::microseconds duration,
                                            bool success)
{
    (void)hook_name;
    (void)duration;
    total_hooks_.fetch_add(1);
    if (!success)
        total_errors_.fetch_add(1);
}

double MetricsCollector::averageLatencyMs() const noexcept
{
    size_t total = total_requests_.load();
    if (total == 0) return 0.0;
    return static_cast<double>(total_latency_us_.load()) / static_cast<double>(total) / 1000.0;
}

nlohmann::json MetricsCollector::snapshot() const
{
    return {
        {"total_requests", total_requests_.load()},
        {"total_errors", total_errors_.load()},
        {"total_hooks_executed", total_hooks_.load()},
        {"average_latency_ms", averageLatencyMs()}
    };
}

} // namespace ohtoai
