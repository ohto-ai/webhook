#pragma once

#include "../metrics/metrics_collector.h"
#include <cpp-httplib/httplib.h>
#include <atomic>

namespace ohtoai {

class HealthEndpoint {
public:
    HealthEndpoint(MetricsCollector& metrics);

    void install(httplib::Server& server);

    void setReady(bool ready) { ready_.store(ready); }
    bool isReady() const { return ready_.load(); }

private:
    MetricsCollector& metrics_;
    std::atomic<bool> ready_{true};
};

} // namespace ohtoai
