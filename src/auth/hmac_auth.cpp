#include "hmac_auth.h"

#include <spdlog/spdlog.h>
#include <cppcodec/hex_default_lower.hpp>

#include <openssl/hmac.h>

namespace ohtoai {

HmacAuthHandler::HmacAuthHandler(const std::string& secret)
    : secret_(secret)
{
}

bool HmacAuthHandler::verify(const std::string& signature_header,
                             const std::string& body) const
{
    if (secret_.empty() || signature_header.empty())
        return false;

    // Expect format: sha256=<hex-encoded-hmac>
    std::string prefix = "sha256=";
    if (signature_header.size() < prefix.size() ||
        signature_header.substr(0, prefix.size()) != prefix)
        return false;

    std::string expected = signature_header.substr(prefix.size());
    std::string computed;

    try {
        computed = computeHmacSha256(body, secret_);
    } catch (const std::exception& e) {
        spdlog::error("HMAC computation failed: {}", e.what());
        return false;
    }

    return constantTimeEquals(computed, expected);
}

std::string HmacAuthHandler::computeHmacSha256(const std::string& data, const std::string& key)
{
    unsigned char result[EVP_MAX_MD_SIZE];
    unsigned int result_len = 0;

    HMAC(EVP_sha256(),
         key.data(), static_cast<int>(key.size()),
         reinterpret_cast<const unsigned char*>(data.data()), data.size(),
         result, &result_len);

    return hex::encode(std::string(reinterpret_cast<char*>(result), result_len));
}

bool HmacAuthHandler::constantTimeEquals(const std::string& a, const std::string& b)
{
    if (a.size() != b.size())
        return false;

    unsigned int result = 0;
    for (size_t i = 0; i < a.size(); ++i) {
        result |= static_cast<unsigned char>(a[i]) ^ static_cast<unsigned char>(b[i]);
    }
    return result == 0;
}

} // namespace ohtoai
