#include "auth_manager.h"

#include <spdlog/spdlog.h>
#include <fplus/fplus.hpp>

namespace ohtoai {

AuthManager::AuthManager(const AuthConfig& config)
    : config_(config)
{
}

httplib::Server::HandlerResponse AuthManager::authenticate(const httplib::Request& req,
                                                            httplib::Response& res)
{
    // Fix client address from trusted proxy headers
    if (req.has_header("X-Real-IP") && !config_.trusted_proxies.empty()) {
        auto& mutable_req = const_cast<httplib::Request&>(req);
        std::string proxy_ip = req.get_header_value("X-Real-IP");
        if (!proxy_ip.empty())
            mutable_req.remote_addr = proxy_ip;
    }

    std::string path = req.path;

    if (requiresBasicAuth(path)) {
        if (!req.has_header("Authorization")) {
            spdlog::warn("Auth failed: missing Authorization header for {}", path);
            res.status = 401;
            res.set_header("WWW-Authenticate", "Basic realm=\"Webhook\"");
            res.set_content("Unauthorized", "text/plain");
            return httplib::Server::HandlerResponse::Handled;
        }

        std::string auth = req.get_header_value("Authorization");
        if (!BasicAuthHandler::verify(auth, config_.username, config_.password)) {
            spdlog::warn("Auth failed: invalid credentials for {}", path);
            res.status = 401;
            res.set_header("WWW-Authenticate", "Basic realm=\"Webhook\"");
            res.set_content("Unauthorized", "text/plain");
            return httplib::Server::HandlerResponse::Handled;
        }
    }

    if (requiresHmacAuth()) {
        std::string signature;
        if (req.has_header("X-Hub-Signature-256")) {
            signature = req.get_header_value("X-Hub-Signature-256");
        } else if (req.has_header("X-Webhook-Signature-256")) {
            signature = req.get_header_value("X-Webhook-Signature-256");
        }

        HmacAuthHandler hmac(config_.webhook_secret);
        if (!hmac.verify(signature, req.body)) {
            spdlog::warn("Auth failed: invalid HMAC signature for {}", path);
            res.status = 403;
            res.set_content("Forbidden", "text/plain");
            return httplib::Server::HandlerResponse::Handled;
        }
    }

    return httplib::Server::HandlerResponse::Unhandled;
}

bool AuthManager::requiresBasicAuth(const std::string& path) const
{
    return !config_.path.empty() &&
           !config_.username.empty() &&
           !config_.password.empty() &&
           fplus::is_prefix_of(config_.path, path);
}

bool AuthManager::requiresHmacAuth() const
{
    return !config_.webhook_secret.empty();
}

} // namespace ohtoai
