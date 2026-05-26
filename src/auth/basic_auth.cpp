#include "basic_auth.h"

#include <cppcodec/base64_default_rfc4648.hpp>

namespace ohtoai {

bool BasicAuthHandler::verify(const std::string& auth_header,
                              const std::string& expected_user,
                              const std::string& expected_pass)
{
    if (auth_header.size() < 6 || auth_header.substr(0, 6) != "Basic ")
        return false;

    std::string encoded = auth_header.substr(6);
    std::string decoded;

    try {
        decoded = base64::decode<std::string>(encoded);
    } catch (...) {
        return false;
    }

    auto colon_pos = decoded.find(':');
    if (colon_pos == std::string::npos)
        return false;

    std::string username = decoded.substr(0, colon_pos);
    std::string password = decoded.substr(colon_pos + 1);

    return constantTimeEquals(username, expected_user) &&
           constantTimeEquals(password, expected_pass);
}

bool BasicAuthHandler::constantTimeEquals(const std::string& a, const std::string& b)
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
