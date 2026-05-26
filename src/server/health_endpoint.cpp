#include "health_endpoint.h"
#include <nlohmann/json.hpp>

namespace ohtoai {

HealthEndpoint::HealthEndpoint(MetricsCollector& metrics)
    : metrics_(metrics)
{
}

void HealthEndpoint::install(httplib::Server& server)
{
    // Liveness: always returns 200 if the server is running
    server.Get("/healthz", [](const httplib::Request&, httplib::Response& res) {
        res.set_content("ok", "text/plain");
    });

    // Readiness: returns 503 during graceful shutdown drain
    server.Get("/readyz", [this](const httplib::Request&, httplib::Response& res) {
        if (ready_.load()) {
            res.set_content("ready", "text/plain");
        } else {
            res.status = 503;
            res.set_content("not ready", "text/plain");
        }
    });

    // Metrics endpoint
    server.Get("/metrics", [this](const httplib::Request&, httplib::Response& res) {
        res.set_content(metrics_.snapshot().dump(2), "application/json");
    });
}

} // namespace ohtoai
