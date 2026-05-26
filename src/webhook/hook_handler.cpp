#include "hook_handler.h"

#include <spdlog/spdlog.h>
#include <spdlog/fmt/fmt.h>

namespace ohtoai {

HookHandler::HookHandler(TemplateEngine& template_engine)
    : template_engine_(template_engine)
{
}

void HookHandler::handle(const Hook& hook,
                          const httplib::Request& req,
                          httplib::Response& res)
{
    spdlog::info("Trigger hook '{}'", hook.name);

    auto data = template_engine_.createRenderData(hook.name, hook.command, req);

    // Render the command with template variables
    std::string rendered_command;
    try {
        rendered_command = template_engine_.render(hook.command, data);
    } catch (const std::exception& e) {
        spdlog::error("Command template rendering failed: {}", e.what());
        res.status = 500;
        res.set_content("Command rendering failed", "text/plain");
        return;
    }

    using nlohmann::literals::operator"" _json_pointer;
    data["/context/rendered_command"_json_pointer] = rendered_command;

    // Execute command asynchronously with timeout
    auto future = command_runner_.runAsync(rendered_command);
    std::string output;

    if (hook.command_timeout > 0) {
        auto timeout = std::chrono::milliseconds(hook.command_timeout);
        if (future.wait_for(timeout) == std::future_status::ready) {
            output = future.get();
            spdlog::info("Command output received");
        } else {
            spdlog::warn("Command timed out after {}ms", hook.command_timeout);
            output.clear();
        }
    } else {
        spdlog::info("Waiting for command output...");
        output = future.get();
    }

    template_engine_.addCommandOutput(data, output);

    // Render result template
    std::string content = fmt::format("{}", fmt::join(hook.result.content, "\n"));

    try {
        auto result = template_engine_.render(content, data);
        res.set_content(result, hook.result.type.c_str());

        if (result.size() > 1024)
            spdlog::debug("Render: \n\r{}...\n", result.substr(0, 1024));
        else
            spdlog::debug("Render: \n\r{}\n", result);
    } catch (const std::exception& e) {
        spdlog::error("Render content failed: {}\n\nContent:\n{}\nData:\n{}",
                      e.what(), content, data.dump(4));
        res.status = 500;
        res.set_content("Render content failed", "text/plain");
    }
}

} // namespace ohtoai
