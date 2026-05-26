#include "server_manager.h"
#include "util/platform.h"
#include "version.hpp"

#include <spdlog/spdlog.h>
#include <spdlog/fmt/bundled/color.h>
#include <spdlog/fmt/fmt.h>

#include <chrono>

namespace ohtoai {

ServerManager::ServerManager(ConfigManager& config_manager)
    : config_manager_(config_manager)
    , health_endpoint_(metrics_)
    , hook_handler_(template_engine_)
{
    // Set up template basic data
    nlohmann::json basic;
    using nlohmann::literals::operator"" _json_pointer;
    basic["/context/app"_json_pointer] = VersionHelper::getInstance().AppName;
    basic["/context/version"_json_pointer] = VersionHelper::getInstance().Version;
    basic["/context/commit_hash"_json_pointer] = VersionHelper::getInstance().CommitHash;
    basic["/context/commit_date"_json_pointer] = VersionHelper::getInstance().CommitDate;
    basic["/context/build_date"_json_pointer] = VersionHelper::getInstance().BuildDate;
    basic["/context/build_time"_json_pointer] = VersionHelper::getInstance().BuildTime;
    basic["/context/platform"_json_pointer] = PlatformHelper::getInstance().getPlatform();

    template_engine_.setBasicData(basic);

    // Set default headers
    httplib::Headers default_headers;
    default_headers.emplace("Server", fmt::format("{} {}", VersionHelper::getInstance().AppName, VersionHelper::getInstance().Version));
    http_server_.server().set_default_headers(default_headers);

    // Initialize sub-components from config
    const auto& cfg = config_manager_.get();

    // Rate limiter
    rate_limiter_ = std::make_unique<RateLimiter>(
        cfg.rate_limit.enabled,
        cfg.rate_limit.max_requests,
        std::chrono::seconds(cfg.rate_limit.window_seconds));

    // Request validator
    request_validator_ = std::make_unique<RequestValidator>(cfg.security.max_body_size);

    // CORS middleware
    CorsSettings cors_settings;
    cors_settings.enabled = cfg.cors.enabled;
    cors_settings.allowed_origins = cfg.cors.allowed_origins;
    cors_settings.allowed_methods = cfg.cors.allowed_methods;
    cors_settings.allowed_headers = cfg.cors.allowed_headers;
    cors_settings.allow_credentials = cfg.cors.allow_credentials;
    cors_settings.max_age_seconds = cfg.cors.max_age_seconds;
    cors_middleware_ = std::make_unique<CorsMiddleware>(cors_settings);

    // Auth manager
    AuthConfig auth_config;
    auth_config.username = cfg.listen.auth.username;
    auth_config.password = cfg.listen.auth.password;
    auth_config.path = cfg.listen.auth.path;
    auth_config.webhook_secret = cfg.security.webhook_secret;
    auth_config.trusted_proxies = cfg.security.trusted_proxies;
    auth_manager_ = std::make_unique<AuthManager>(auth_config);
}

ServerManager::~ServerManager()
{
    stop();
}

void ServerManager::welcome() const
{
    auto& ph = PlatformHelper::getInstance();
    auto& vh = VersionHelper::getInstance();

    fmt::print(fg(fmt::color::gold), "{}\n", vh.AsciiBanner);
    fmt::print(fg(fmt::color::green), "\r{:=^{}}\n", "=", ph.getTerminalWidth());

    if (vh.IsDevVersion) {
        fmt::print(fg(fmt::color::red), "This is a development version, please do not use it in production environment.\n");
        fmt::print("Version {} on {}\n", vh.CommitHash, vh.CommitDate);
    } else {
        fmt::print("Version {}({}) on {}\n", vh.Version, vh.CommitHash, vh.CommitDate);
    }

    fmt::print("Build on {} {}\n", vh.BuildDate, vh.BuildTime);
    fmt::print("Run on {} | {}\n", ph.getPlatform(), ph.getCpuInfo());
    fmt::print(fg(fmt::color::green), "\r{:=^{}}\n", "=", ph.getTerminalWidth());
}

ExitReason ServerManager::start()
{
    welcome();

    const auto& cfg = config_manager_.get();

    installPreRouting();
    installHooks();
    installHealthAndFavicon();

    // Set up access logging
    http_server_.setLogger([this](const httplib::Request& req, const httplib::Response& res) {
        spdlog::info("{} {} {} {} bytes User-Agent: {}",
                     req.remote_addr, req.method, req.path,
                     res.body.size(), req.get_header_value("User-Agent"));
        for (const auto& header : req.headers)
            spdlog::debug("Header: {}={}", header.first, header.second);

        fmt::print(fg(fmt::color::green), "\r{:=^{}}\n", " Done ",
                   PlatformHelper::getInstance().getTerminalWidth());
    });

    // Bind and listen
    http_server_.bind(cfg.listen.host.c_str(), cfg.listen.port);

    if (cfg.tls.enabled)
        http_server_.enableTls(cfg.tls.cert_file, cfg.tls.key_file);

    spdlog::info("Server listening on {}:{}", cfg.listen.host, cfg.listen.port);

    // Start config watcher
    setupConfigWatcher();

    health_endpoint_.setReady(true);
    spdlog::info("Server started.");

    bool listen_ok = http_server_.start();

    // Stop watcher
    if (config_watcher_thread_ && config_watcher_thread_->joinable()) {
        config_watcher_thread_->join();
        config_watcher_thread_.reset();
    }

    if (!listen_ok)
        return ExitReason::Terminate;

    return exit_reason_.load();
}

void ServerManager::stop()
{
    health_endpoint_.setReady(false);
    http_server_.stop();
}

void ServerManager::setupConfigWatcher()
{
    config_watcher_thread_ = std::make_unique<std::thread>([this]() {
        while (http_server_.isRunning()) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            if (config_manager_.hasChanged()) {
                spdlog::info("Config file changed, reload.");
                exit_reason_.store(ExitReason::Reload);
                http_server_.stop();
                return;
            }
        }
    });
}

void ServerManager::installPreRouting()
{
    http_server_.server().set_pre_routing_handler([this](const auto& req, auto& res) {
        return preRoutingChain(req, res);
    });
}

httplib::Server::HandlerResponse ServerManager::preRoutingChain(const httplib::Request& req,
                                                                 httplib::Response& res)
{
    // 1. Request validation (body size, path traversal, etc.)
    auto result = request_validator_->validate(req, res);
    if (result == httplib::Server::HandlerResponse::Handled)
        return result;

    // 2. CORS handling
    result = cors_middleware_->handle(req, res);
    if (result == httplib::Server::HandlerResponse::Handled)
        return result;

    // 3. Rate limiting
    if (!rate_limiter_->allow(req.remote_addr)) {
        int retry_after = rate_limiter_->retryAfterSeconds(req.remote_addr);
        spdlog::warn("Rate limit exceeded for {}", req.remote_addr);
        res.status = 429;
        res.set_header("Retry-After", std::to_string(retry_after));
        res.set_content("Too Many Requests", "text/plain");
        return httplib::Server::HandlerResponse::Handled;
    }

    // 4. Authentication
    result = auth_manager_->authenticate(req, res);
    if (result == httplib::Server::HandlerResponse::Handled)
        return result;

    // 5. Record metrics
    auto start = std::chrono::steady_clock::now();
    metrics_.recordRequest({
        req.method,
        req.path,
        200, // Will be updated by logger if different
        std::chrono::duration_cast<std::chrono::microseconds>(
            std::chrono::steady_clock::now() - start)
    });

    return httplib::Server::HandlerResponse::Unhandled;
}

void ServerManager::installHooks()
{
    const auto& cfg = config_manager_.get();

    for (const auto& hook : cfg.hooks) {
        auto path = fmt::format("{}{}", cfg.listen.prefix, hook.path);
        spdlog::info("Bind '{}' {} {} hook, with command '{}'",
                     hook.name, hook.method, path, hook.command);

        auto handler = [this, &hook](const httplib::Request& req, httplib::Response& res) {
            hook_handler_.handle(hook, req, res);
        };

        if (hook.method == "GET")
            http_server_.server().Get(path.c_str(), handler);
        else if (hook.method == "POST")
            http_server_.server().Post(path.c_str(), handler);
        else if (hook.method == "DELETE")
            http_server_.server().Delete(path.c_str(), handler);
        else if (hook.method == "PUT")
            http_server_.server().Put(path.c_str(), handler);
        else if (hook.method == "PATCH")
            http_server_.server().Patch(path.c_str(), handler);
        else if (hook.method == "OPTIONS")
            http_server_.server().Options(path.c_str(), handler);
        else
            spdlog::error("Illegal method: {}", hook.method);
    }
}

void ServerManager::installHealthAndFavicon()
{
    health_endpoint_.install(http_server_.server());

    // Default favicon
    constexpr auto favicon_svg = R"FAVICON(<svg class="icon" style="width: 1em;height: 1em;vertical-align: middle;fill: currentColor;overflow: hidden;" viewBox="0 0 1024 1024" version="1.1" xmlns="http://www.w3.org/2000/svg"><path d="M96.228571 697.828571z m121.942858-105.485714c-28.571429 28.571429-44.228571 66.4-44.228572 106.742857s15.771429 78.285714 44.228572 106.742857c28.571429 28.571429 66.4 44.228571 106.742857 44.228572 40.342857 0 78.285714-15.771429 106.742857-44.228572l67.885714-67.885714-213.485714-213.485714-67.885714 67.885714z m480.914285-418.4c-40.342857 0-78.285714 15.771429-106.742857 44.228572l-67.885714 67.885714 213.485714 213.485714 67.885714-67.885714c28.457143-28.571429 44.228571-66.4 44.228572-106.742857s-15.771429-78.285714-44.228572-106.742857c-28.571429-28.571429-66.4-44.228571-106.742857-44.228572z" fill="#D9D9D9" /><path d="M588.457143 551.657143a9.177143 9.177143 0 0 0-12.914286 0L499.428571 627.771429 396.228571 524.571429l76.228572-76.228572c3.542857-3.542857 3.542857-9.371429 0-12.914286L430.857143 393.828571a9.177143 9.177143 0 0 0-12.914286 0L341.714286 470.057143l-49.142857-49.142857a8.971429 8.971429 0 0 0-6.514286-2.628572c-2.285714 0-4.685714 0.914286-6.514286 2.628572L163.2 537.371429a227.942857 227.942857 0 0 0-66.971429 160.457142c-0.228571 45.142857 12.8 90.4 39.2 129.257143l-86.971428 86.971429a9.177143 9.177143 0 0 0 0 12.914286l48.457143 48.457142c1.828571 1.828571 4.114286 2.628571 6.514285 2.628572s4.685714-0.914286 6.514286-2.628572l86.971429-86.971428c38.514286 26.171429 83.314286 39.2 128.114285 39.2 58.514286 0 117.028571-22.285714 161.714286-66.971429l116.457143-116.457143c3.542857-3.542857 3.542857-9.371429 0-12.914285l-49.142857-49.142857 76.228571-76.228572c3.542857-3.542857 3.542857-9.371429 0-12.914286l-41.828571-41.371428zM431.657143 805.828571a150.08 150.08 0 0 1-106.742857 44.228572c-40.342857 0-78.171429-15.657143-106.742857-44.228572-28.457143-28.457143-44.228571-66.4-44.228572-106.742857s15.657143-78.171429 44.228572-106.742857l67.885714-67.885714 213.485714 213.485714-67.885714 67.885714z m544-708.914285l-48.457143-48.457143c-1.828571-1.828571-4.114286-2.628571-6.514286-2.628572s-4.685714 0.914286-6.514285 2.628572l-86.971429 86.971428a227.737143 227.737143 0 0 0-128.114286-39.2c-58.514286 0-117.028571 22.285714-161.714285 66.971429L420.914286 279.657143a9.177143 9.177143 0 0 0 0 12.914286L731.428571 603.085714c1.828571 1.828571 4.114286 2.628571 6.514286 2.628572 2.285714 0 4.685714-0.914286 6.514286-2.628572l116.457143-116.457143c78.742857-78.857143 88-200.8 27.771428-289.714285l86.971429-86.971429c3.542857-3.657143 3.542857-9.485714 0-13.028571zM805.828571 431.657143l-67.885714 67.885714-213.485714-213.485714 67.885714-67.885714c28.457143-28.457143 66.4-44.228571 106.742857-44.228572s78.171429 15.657143 106.742857 44.228572c28.457143 28.457143 44.228571 66.4 44.228572 106.742857s-15.771429 78.171429-44.228572 106.742857z"  /></svg>)FAVICON";

    http_server_.server().Get("/favicon.ico", [favicon_svg](const httplib::Request&, httplib::Response& res) {
        res.set_content(favicon_svg, "image/svg+xml");
    });
}

} // namespace ohtoai
