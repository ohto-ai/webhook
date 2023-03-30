#pragma once

#ifndef OHTOAI_WEBHOOK_MANAGER_H
#define OHTOAI_WEBHOOK_MANAGER_H

#include "config/file_configurator.hpp"
#include "config/config_modal.hpp"

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
    private:
        FileConfigurator configurator;
        WebhookConfigModal config;
        httplib::Server server;
    };
} // namespace ohtoai

#endif
