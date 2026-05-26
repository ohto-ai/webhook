#pragma once

#include <string>
#include <chrono>
#include <atomic>
#include <mutex>
#include <nlohmann/json.hpp>

namespace ohtoai {

struct RequestRecord {
    std::string method;
    std::string path;
    int status;
    std::chrono::microseconds duration;
};

class MetricsCollector {
public:
    void recordRequest(const RequestRecord& record);
    void recordHookExecution(const std::string& hook_name,
                             std::chrono::microseconds duration,
                             bool success);

    size_t totalRequests() const noexcept { return total_requests_.load(); }
    size_t totalErrors() const noexcept { return total_errors_.load(); }
    size_t totalHooksExecuted() const noexcept { return total_hooks_.load(); }
    double averageLatencyMs() const noexcept;
    nlohmann::json snapshot() const;

private:
    std::atomic<size_t> total_requests_{0};
    std::atomic<size_t> total_errors_{0};
    std::atomic<size_t> total_hooks_{0};
    std::atomic<int64_t> total_latency_us_{0};
};

} // namespace ohtoai
