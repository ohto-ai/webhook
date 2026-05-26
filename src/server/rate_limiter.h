#pragma once

#include <string>
#include <chrono>
#include <mutex>
#include <unordered_map>

namespace ohtoai {

class RateLimiter {
public:
    RateLimiter(bool enabled, size_t max_requests, std::chrono::seconds window);
    explicit RateLimiter();

    bool allow(const std::string& client_ip);
    size_t remaining(const std::string& client_ip) const;
    int retryAfterSeconds(const std::string& client_ip) const;
    bool enabled() const { return enabled_; }

private:
    struct TokenBucket {
        double tokens;
        std::chrono::steady_clock::time_point last_refill;
    };

    double refillRate() const;

    bool enabled_ = false;
    double max_tokens_ = 100.0;
    std::chrono::seconds window_{60};
    mutable std::mutex mutex_;
    mutable std::unordered_map<std::string, TokenBucket> buckets_;
};

} // namespace ohtoai
