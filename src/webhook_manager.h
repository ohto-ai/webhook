#pragma once

#ifndef OHTOAI_WEBHOOK_MANAGER_H
#define OHTOAI_WEBHOOK_MANAGER_H

namespace ohtoai {
    class WebhookManager {
    public:
        WebhookManager(int argc, char **argv);
        ~WebhookManager();
        int exec();
    };
} // namespace ohtoai

#endif
