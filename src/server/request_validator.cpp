#include "request_validator.h"
#include <spdlog/spdlog.h>
#include <algorithm>

namespace ohtoai {

RequestValidator::RequestValidator(size_t max_body_size)
    : max_body_size_(max_body_size)
{
}

httplib::Server::HandlerResponse RequestValidator::validate(const httplib::Request& req,
                                                             httplib::Response& res)
{
    if (isPathTraversal(req.path)) {
        spdlog::warn("Rejected path traversal attempt: {}", req.path);
        res.status = 400;
        res.set_content("Bad Request", "text/plain");
        return httplib::Server::HandlerResponse::Handled;
    }

    if (req.body.size() > max_body_size_) {
        spdlog::warn("Request body too large: {} bytes (max {})", req.body.size(), max_body_size_);
        res.status = 413;
        res.set_content("Payload Too Large", "text/plain");
        return httplib::Server::HandlerResponse::Handled;
    }

    return httplib::Server::HandlerResponse::Unhandled;
}

bool RequestValidator::isPathTraversal(const std::string& path) const
{
    return path.find("..") != std::string::npos ||
           path.find("%2e%2e") != std::string::npos ||
           path.find("%2E%2E") != std::string::npos;
}

} // namespace ohtoai
