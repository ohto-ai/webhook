#pragma once

#include <cpp-httplib/httplib.h>
#include <string>
#include <functional>

namespace ohtoai {

class HttpServer {
public:
    HttpServer();
    ~HttpServer();

    httplib::Server& server() { return server_; }

    bool bind(const std::string& host, int port);
    bool enableTls(const std::string& cert_file, const std::string& key_file);
    bool tlsEnabled() const { return tls_enabled_; }

    bool start();
    void stop();

    bool isRunning() const { return server_.is_running(); }

    void setLogger(std::function<void(const httplib::Request&, const httplib::Response&)> logger);

private:
    httplib::Server server_;
    bool tls_enabled_ = false;
    std::string cert_file_;
    std::string key_file_;
    std::string bind_host_;
    int bind_port_ = 0;
};

} // namespace ohtoai
