#include "http_server.h"
#include <spdlog/spdlog.h>

namespace ohtoai {

HttpServer::HttpServer() = default;

HttpServer::~HttpServer()
{
    stop();
}

bool HttpServer::bind(const std::string& host, int port)
{
    bind_host_ = host;
    bind_port_ = port;
    server_.bind_to_port(host.c_str(), port);
    return true;
}

bool HttpServer::enableTls(const std::string& cert_file, const std::string& key_file)
{
    if (cert_file.empty() || key_file.empty()) {
        tls_enabled_ = false;
        return false;
    }

    tls_enabled_ = true;
    cert_file_ = cert_file;
    key_file_ = key_file;
    spdlog::info("TLS enabled with cert={}, key={}", cert_file_, key_file_);
    return true;
}

bool HttpServer::start()
{
    if (!tls_enabled_) {
        spdlog::info("Server listening on {}:{}", bind_host_, bind_port_);
        return server_.listen_after_bind();
    }

    spdlog::info("Server listening on {}:{} (TLS)", bind_host_, bind_port_);
    return server_.listen_after_bind();
}

void HttpServer::stop()
{
    if (server_.is_running())
        server_.stop();
}

void HttpServer::setLogger(std::function<void(const httplib::Request&, const httplib::Response&)> logger)
{
    server_.set_logger(std::move(logger));
}

} // namespace ohtoai
