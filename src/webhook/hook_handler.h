#pragma once

#include "template_engine.h"
#include "../command/command_runner.h"
#include "../config/config_model.hpp"

#include <cpp-httplib/httplib.h>
#include <memory>

namespace ohtoai {

class HookHandler {
public:
    HookHandler(TemplateEngine& template_engine);

    void handle(const Hook& hook,
                const httplib::Request& req,
                httplib::Response& res);

private:
    CommandRunner command_runner_;
    TemplateEngine& template_engine_;
};

} // namespace ohtoai
