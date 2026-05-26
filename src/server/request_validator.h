#pragma once

#include <string>
#include <vector>
#include <cpp-httplib/httplib.h>

namespace ohtoai {

class RequestValidator {
public:
    explicit RequestValidator(size_t max_body_size = 1048576);

    // Returns Handled (with 4xx) if invalid, Unhandled if OK
    httplib::Server::HandlerResponse validate(const httplib::Request& req,
                                               httplib::Response& res);

private:
    bool isPathTraversal(const std::string& path) const;

    size_t max_body_size_;
};

} // namespace ohtoai
