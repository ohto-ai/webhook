#include <catch2/catch.hpp>
#include <server/rate_limiter.h>

using namespace std::chrono_literals;
using ohtoai::RateLimiter;

TEST_CASE("RateLimiter: allows requests when disabled", "[rate_limit]")
{
    RateLimiter limiter;
    // Not enabled by default
    REQUIRE(limiter.allow("127.0.0.1"));
    REQUIRE(limiter.allow("127.0.0.1"));
    REQUIRE(limiter.allow("127.0.0.1"));
}

TEST_CASE("RateLimiter: allows within limit", "[rate_limit]")
{
    RateLimiter limiter(true, 5, 60s);

    for (int i = 0; i < 5; ++i)
        REQUIRE(limiter.allow("127.0.0.1"));
}

TEST_CASE("RateLimiter: blocks over limit", "[rate_limit]")
{
    RateLimiter limiter(true, 3, 60s);

    REQUIRE(limiter.allow("127.0.0.1"));
    REQUIRE(limiter.allow("127.0.0.1"));
    REQUIRE(limiter.allow("127.0.0.1"));
    REQUIRE_FALSE(limiter.allow("127.0.0.1"));
}

TEST_CASE("RateLimiter: separate IPs tracked independently", "[rate_limit]")
{
    RateLimiter limiter(true, 2, 60s);

    REQUIRE(limiter.allow("192.168.1.1"));
    REQUIRE(limiter.allow("192.168.1.1"));
    REQUIRE_FALSE(limiter.allow("192.168.1.1"));

    REQUIRE(limiter.allow("10.0.0.1"));
    REQUIRE(limiter.allow("10.0.0.1"));
    REQUIRE_FALSE(limiter.allow("10.0.0.1"));
}

TEST_CASE("RateLimiter: reports remaining tokens", "[rate_limit]")
{
    RateLimiter limiter(true, 5, 60s);

    limiter.allow("127.0.0.1");
    limiter.allow("127.0.0.1");

    REQUIRE(limiter.remaining("127.0.0.1") <= 3);
}

TEST_CASE("RateLimiter: retry after seconds", "[rate_limit]")
{
    RateLimiter limiter(true, 1, 60s);

    limiter.allow("127.0.0.1");
    REQUIRE_FALSE(limiter.allow("127.0.0.1"));

    int retry = limiter.retryAfterSeconds("127.0.0.1");
    REQUIRE(retry > 0);
}

TEST_CASE("RateLimiter: disabled state", "[rate_limit]")
{
    RateLimiter limiter(true, 0, 60s);
    // With 0 max requests and disabled, should still allow
    RateLimiter disabled;
    REQUIRE(disabled.allow("127.0.0.1"));
}
