#pragma once

#include <string>
#include <cpp-httplib/httplib.h>

namespace ohtoai {

struct CorsSettings {
    bool enabled = false;
    std::string allowed_origins = "*";
    std::string allowed_methods = "GET, POST, PUT, DELETE, PATCH, OPTIONS";
    std::string allowed_headers = "Content-Type, Authorization";
    bool allow_credentials = false;
    int max_age_seconds = 86400;
};

class CorsMiddleware {
public:
    explicit CorsMiddleware(const CorsSettings& settings);

    // Returns Handled for preflight OPTIONS requests, Unhandled otherwise
    httplib::Server::HandlerResponse handle(const httplib::Request& req,
                                             httplib::Response& res);

private:
    void injectCorsHeaders(httplib::Response& res);
    CorsSettings settings_;
};

} // namespace ohtoai
