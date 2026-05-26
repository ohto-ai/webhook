#pragma once

#include "config/config_manager.h"
#include "auth/auth_manager.h"
#include "server/http_server.h"
#include "server/rate_limiter.h"
#include "server/request_validator.h"
#include "server/cors_middleware.h"
#include "server/health_endpoint.h"
#include "webhook/template_engine.h"
#include "webhook/hook_handler.h"
#include "metrics/metrics_collector.h"

#include <memory>
#include <thread>
#include <atomic>
#include <filesystem>

namespace ohtoai {

enum class ExitReason { Finish, Reload, Terminate };

class ServerManager {
public:
    ServerManager(ConfigManager& config_manager);
    ~ServerManager();

    ExitReason start();
    void stop();

private:
    void welcome() const;
    void installPreRouting();
    void installHooks();
    void installHealthAndFavicon();
    void setupConfigWatcher();
    httplib::Server::HandlerResponse preRoutingChain(const httplib::Request& req,
                                                       httplib::Response& res);

    ConfigManager& config_manager_;
    HttpServer http_server_;
    TemplateEngine template_engine_;
    MetricsCollector metrics_;
    HealthEndpoint health_endpoint_;
    HookHandler hook_handler_;

    std::unique_ptr<AuthManager> auth_manager_;
    std::unique_ptr<RateLimiter> rate_limiter_;
    std::unique_ptr<RequestValidator> request_validator_;
    std::unique_ptr<CorsMiddleware> cors_middleware_;

    std::unique_ptr<std::thread> config_watcher_thread_;
    std::atomic<ExitReason> exit_reason_{ExitReason::Finish};
};

} // namespace ohtoai
