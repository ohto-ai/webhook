#pragma once

#include <string>

namespace ohtoai {

class BasicAuthHandler {
public:
    static bool verify(const std::string& auth_header,
                       const std::string& expected_user,
                       const std::string& expected_pass);

private:
    static bool constantTimeEquals(const std::string& a, const std::string& b);
};

} // namespace ohtoai
