#include <catch2/catch.hpp>
#include <auth/hmac_auth.h>
#include <openssl/hmac.h>
#include <cppcodec/hex_default_lower.hpp>

using ohtoai::HmacAuthHandler;

static std::string computeSignature(const std::string& secret, const std::string& body)
{
    unsigned char result[EVP_MAX_MD_SIZE];
    unsigned int result_len = 0;
    HMAC(EVP_sha256(),
         secret.data(), static_cast<int>(secret.size()),
         reinterpret_cast<const unsigned char*>(body.data()), body.size(),
         result, &result_len);
    return "sha256=" + hex::encode(std::string(reinterpret_cast<char*>(result), result_len));
}

TEST_CASE("HMAC: valid signature", "[auth]")
{
    HmacAuthHandler handler("my-secret-key");
    std::string body = R"({"action": "push", "ref": "refs/heads/main"})";
    std::string signature = computeSignature("my-secret-key", body);

    REQUIRE(handler.verify(signature, body));
}

TEST_CASE("HMAC: wrong secret", "[auth]")
{
    HmacAuthHandler handler("my-secret-key");
    std::string body = R"({"action": "push"})";
    std::string signature = computeSignature("different-secret", body);

    REQUIRE_FALSE(handler.verify(signature, body));
}

TEST_CASE("HMAC: tampered body", "[auth]")
{
    HmacAuthHandler handler("my-secret-key");
    std::string original_body = R"({"action": "push"})";
    std::string signature = computeSignature("my-secret-key", original_body);
    std::string tampered_body = R"({"action": "delete"})";

    REQUIRE_FALSE(handler.verify(signature, tampered_body));
}

TEST_CASE("HMAC: empty secret disables verification", "[auth]")
{
    HmacAuthHandler handler("");
    REQUIRE_FALSE(handler.enabled());
}

TEST_CASE("HMAC: non-empty secret enables verification", "[auth]")
{
    HmacAuthHandler handler("secret");
    REQUIRE(handler.enabled());
}

TEST_CASE("HMAC: empty signature rejected", "[auth]")
{
    HmacAuthHandler handler("secret");
    REQUIRE_FALSE(handler.verify("", "body"));
}

TEST_CASE("HMAC: missing sha256 prefix", "[auth]")
{
    HmacAuthHandler handler("secret");
    REQUIRE_FALSE(handler.verify("not-sha256=abc123", "body"));
}
