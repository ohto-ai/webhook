#include "cors_middleware.h"

namespace ohtoai {

CorsMiddleware::CorsMiddleware(const CorsSettings& settings)
    : settings_(settings)
{
}

httplib::Server::HandlerResponse CorsMiddleware::handle(const httplib::Request& req,
                                                         httplib::Response& res)
{
    if (!settings_.enabled)
        return httplib::Server::HandlerResponse::Unhandled;

    injectCorsHeaders(res);

    // Handle preflight
    if (req.method == "OPTIONS") {
        res.status = 204;
        return httplib::Server::HandlerResponse::Handled;
    }

    return httplib::Server::HandlerResponse::Unhandled;
}

void CorsMiddleware::injectCorsHeaders(httplib::Response& res)
{
    res.set_header("Access-Control-Allow-Origin", settings_.allowed_origins);
    res.set_header("Access-Control-Allow-Methods", settings_.allowed_methods);
    res.set_header("Access-Control-Allow-Headers", settings_.allowed_headers);
    res.set_header("Access-Control-Max-Age", std::to_string(settings_.max_age_seconds));

    if (settings_.allow_credentials)
        res.set_header("Access-Control-Allow-Credentials", "true");
}

} // namespace ohtoai
