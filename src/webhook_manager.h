#pragma once

#ifndef OHTOAI_WEBHOOK_MANAGER_H
#define OHTOAI_WEBHOOK_MANAGER_H

#include "config/config_modal.hpp"

#include <inja/inja.hpp>
#include <cpp-httplib/httplib.h>

#include <thread>
#include <memory>
#include <filesystem>

namespace ohtoai {

    enum ExitReason {
        Finish,
        Reload,
        Terminate     
    };

    class WebhookManager {
    public:
        WebhookManager(int argc, char **argv);
        ~WebhookManager();
        ExitReason exec();

    private:
        void welcome() const;
        bool serve_precondition();
        bool doLoadConfig();
        bool installLoggers();
        bool installHooks();

        void fillEnv(inja::Environment& env, nlohmann::json & data, const Hook& hook, const httplib::Request &req, httplib::Response &res);

        httplib::Server::HandlerResponse authRoutingHandler(const httplib::Request &req, httplib::Response &res);

    private:
        WebhookConfigModal config;
        httplib::Server server;
        inja::Environment inja_env;
        nlohmann::json basic_render_data;
        httplib::Headers default_headers;
        std::unique_ptr<std::thread> config_monitor_thread;
        std::filesystem::file_time_type last_config_modify_time;
        ExitReason exit_reason {ExitReason::Finish};

        bool arg_generate_config_if_not_exist {true};
        bool arg_quit_after_config_generate {true};
        std::filesystem::path arg_config_path {"hook.json"};
    };
} // namespace ohtoai

#endif
