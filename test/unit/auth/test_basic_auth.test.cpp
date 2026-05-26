#include <catch2/catch.hpp>
#include <auth/basic_auth.h>
#include <cppcodec/base64_default_rfc4648.hpp>

using ohtoai::BasicAuthHandler;

TEST_CASE("BasicAuth: valid credentials", "[auth]")
{
    std::string user = "admin";
    std::string pass = "secret123";
    std::string credentials = user + ":" + pass;
    std::string encoded = base64::encode(credentials);
    std::string header = "Basic " + encoded;

    REQUIRE(BasicAuthHandler::verify(header, "admin", "secret123"));
}

TEST_CASE("BasicAuth: wrong password", "[auth]")
{
    std::string credentials = "admin:wrongpass";
    std::string encoded = base64::encode(credentials);
    std::string header = "Basic " + encoded;

    REQUIRE_FALSE(BasicAuthHandler::verify(header, "admin", "secret123"));
}

TEST_CASE("BasicAuth: wrong username", "[auth]")
{
    std::string credentials = "hacker:secret123";
    std::string encoded = base64::encode(credentials);
    std::string header = "Basic " + encoded;

    REQUIRE_FALSE(BasicAuthHandler::verify(header, "admin", "secret123"));
}

TEST_CASE("BasicAuth: missing Basic prefix", "[auth]")
{
    std::string credentials = "admin:secret123";
    std::string encoded = base64::encode(credentials);
    std::string header = encoded; // No "Basic " prefix

    REQUIRE_FALSE(BasicAuthHandler::verify(header, "admin", "secret123"));
}

TEST_CASE("BasicAuth: empty header", "[auth]")
{
    REQUIRE_FALSE(BasicAuthHandler::verify("", "admin", "secret123"));
}

TEST_CASE("BasicAuth: malformed base64", "[auth]")
{
    std::string header = "Basic !!!not-valid-base64!!!";

    REQUIRE_FALSE(BasicAuthHandler::verify(header, "admin", "secret123"));
}

TEST_CASE("BasicAuth: no colon separator", "[auth]")
{
    std::string encoded = base64::encode("nocolon");
    std::string header = "Basic " + encoded;

    REQUIRE_FALSE(BasicAuthHandler::verify(header, "admin", "secret123"));
}

TEST_CASE("BasicAuth: special characters in password", "[auth]")
{
    std::string user = "admin";
    std::string pass = "p@ss:word!";
    std::string credentials = user + ":" + pass;
    std::string encoded = base64::encode(credentials);
    std::string header = "Basic " + encoded;

    REQUIRE(BasicAuthHandler::verify(header, "admin", "p@ss:word!"));
}
