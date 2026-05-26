#include "rate_limiter.h"

namespace ohtoai {

RateLimiter::RateLimiter(bool enabled, size_t max_requests, std::chrono::seconds window)
    : enabled_(enabled)
    , max_tokens_(static_cast<double>(max_requests))
    , window_(window)
{
}

RateLimiter::RateLimiter()
    : enabled_(false)
{
}

double RateLimiter::refillRate() const
{
    return static_cast<double>(max_tokens_) / static_cast<double>(window_.count());
}

bool RateLimiter::allow(const std::string& client_ip)
{
    if (!enabled_)
        return true;

    std::lock_guard<std::mutex> lock(mutex_);
    auto now = std::chrono::steady_clock::now();
    auto& bucket = buckets_[client_ip];

    // Initialize bucket on first request
    if (bucket.tokens == 0.0 && bucket.last_refill == std::chrono::steady_clock::time_point{}) {
        bucket.tokens = max_tokens_;
        bucket.last_refill = now;
    }

    // Refill tokens based on elapsed time
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - bucket.last_refill).count();
    double refill = elapsed * refillRate() / 1000.0;
    bucket.tokens = std::min(max_tokens_, bucket.tokens + refill);
    bucket.last_refill = now;

    if (bucket.tokens >= 1.0) {
        bucket.tokens -= 1.0;
        return true;
    }

    bucket.tokens = 0.0;
    return false;
}

size_t RateLimiter::remaining(const std::string& client_ip) const
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = buckets_.find(client_ip);
    if (it == buckets_.end())
        return static_cast<size_t>(max_tokens_);
    return static_cast<size_t>(it->second.tokens);
}

int RateLimiter::retryAfterSeconds(const std::string& client_ip) const
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = buckets_.find(client_ip);
    if (it == buckets_.end())
        return 0;

    double deficit = 1.0 - it->second.tokens;
    if (deficit <= 0.0)
        return 0;

    double seconds = deficit / refillRate();
    return static_cast<int>(std::ceil(seconds));
}

} // namespace ohtoai
