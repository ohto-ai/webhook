#pragma once

#ifndef OHTOAI_WEBHOOK_MANAGER_H
#define OHTOAI_WEBHOOK_MANAGER_H

#include "config/file_configurator.hpp"
#include "config/config_modal.hpp"

#include <inja/inja.hpp>
#include <cpp-httplib/httplib.h>

namespace ohtoai {
    class WebhookManager {
    public:
        WebhookManager(int argc, char **argv);
        ~WebhookManager();
        int exec();
    private:
        void welcome() const;
        bool serve_precondition();
        bool doLoadConfig();
        bool installLoggers();
        bool installHooks();

        std::tuple<inja::Environment&&, nlohmann::json &&> fillEnv(const Hook& hook, const httplib::Request &req, httplib::Response &res);

        httplib::Server::HandlerResponse authRoutingHandler(const httplib::Request &req, httplib::Response &res);
    private:
        FileConfigurator configurator;
        WebhookConfigModal config;
        httplib::Server server;
        inja::Environment inja_env;
        nlohmann::json basic_render_data;
        httplib::Headers default_headers;
    };
} // namespace ohtoai

#endif
