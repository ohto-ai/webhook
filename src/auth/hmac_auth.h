#pragma once

#include <string>

namespace ohtoai {

class HmacAuthHandler {
public:
    explicit HmacAuthHandler(const std::string& secret);

    bool verify(const std::string& signature_header,
                const std::string& body) const;

    bool enabled() const { return !secret_.empty(); }

private:
    std::string secret_;
    static std::string computeHmacSha256(const std::string& data, const std::string& key);
    static bool constantTimeEquals(const std::string& a, const std::string& b);
};

} // namespace ohtoai
