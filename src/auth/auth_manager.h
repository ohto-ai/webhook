#pragma once

#include "basic_auth.h"
#include "hmac_auth.h"

#include <cpp-httplib/httplib.h>
#include <string>
#include <vector>

namespace ohtoai {

struct AuthConfig {
    std::string username;
    std::string password;
    std::string path = "/";
    std::string webhook_secret;
    std::vector<std::string> trusted_proxies = {"127.0.0.1", "::1"};
};

class AuthManager {
public:
    explicit AuthManager(const AuthConfig& config);

    // Returns Handled if auth failed (401), Unhandled if passed
    httplib::Server::HandlerResponse authenticate(const httplib::Request& req,
                                                   httplib::Response& res);

private:
    bool requiresBasicAuth(const std::string& path) const;
    bool requiresHmacAuth() const;
    std::string getClientAddress(const httplib::Request& req) const;

    AuthConfig config_;
};

} // namespace ohtoai
