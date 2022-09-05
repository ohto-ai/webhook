#include <cpp-httplib/httplib.h>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <fstream>
#include <algorithm>
#include <iterator>
#include <brynet/base/crypto/Base64.hpp>
#include "ohtoai/string_tools.hpp"

#ifdef _WIN32
#define popen _popen
#define pclose _pclose
#endif

std::string executeCommand(std::string cmd)
{
	std::string display{};
	auto f = popen(cmd.c_str(), "r");
	char buf_ps[1024];
	if (f != nullptr)
	{
		while (fgets(buf_ps, 1024, f) != nullptr)
			display += buf_ps;
		pclose(f);
	}
	else
	{
		spdlog::error("Cannot open pip `{}`", cmd);
	}
	return display;
}

struct UserLogin {
	std::string username;
	std::string password;
};

int main()
{
	constexpr auto configPath{ "hook.json" };

	spdlog::info("Application start.");
	spdlog::info("Load config {}", configPath);
	std::ifstream ifs(configPath);
	if (!ifs) {
		spdlog::error("Failed to open hook.json");
		return -1;
	}

	nlohmann::json configJ;
	try {
		ifs >> configJ;
	}
	catch (nlohmann::detail::exception e)
	{
		spdlog::error("{}", e.what());
		return -1;
	}

	ifs.close();

	std::string log_console_level = configJ["log"]["console_level"];
	std::string log_file_level = configJ["log"]["file_level"];
	std::string log_file_path = configJ["log"]["file_path"];
	std::string log_global_level = configJ["log"]["global_level"];

	try
	{
		auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
		console_sink->set_level(spdlog::level::from_str(log_console_level));

		auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>("hook.log", true);
		file_sink->set_level(spdlog::level::from_str(log_file_level));

		spdlog::set_default_logger(std::make_shared<spdlog::logger>("webhook", spdlog::sinks_init_list({ console_sink, file_sink })));
		spdlog::set_level(spdlog::level::from_str(log_global_level));
	}
	catch (const spdlog::spdlog_ex& ex)
	{
		std::cout << "Log initialization failed: " << ex.what() << std::endl;
	}

	spdlog::info("Config loaded.");

	httplib::Server server;

	UserLogin globalUserLogin{ configJ["auth"]["username"], configJ["auth"]["password"] };
	bool isAuthEnabled = configJ["auth"]["enabled"];
	std::string pathPrefix{};
	if (configJ["listen"].contains("prefix"))
		pathPrefix = configJ["listem"]["prefix"];

	server.bind_to_port(configJ["listen"]["host"].get<std::string>().c_str(), configJ["listen"]["port"]);

	server.set_pre_routing_handler([&server, &globalUserLogin, isAuthEnabled, pathPrefix](const httplib::Request& req, httplib::Response& res)
		{
			if (!ohtoai::tool::string::start_with(req.path, pathPrefix))
			{
				spdlog::warn("Refuse to path {}", req.path);
				return httplib::Server::HandlerResponse::Unhandled;
			}

			spdlog::info("Routing {} {} \nReceive {} bytes\n{}", req.method, req.path, req.body.size(), req.body);

			if (isAuthEnabled)
			{
				if (req.method == "GET")
				{
					if (!req.has_header("Authorization") || req.get_header_value("Authorization").empty())
					{
						res.set_header("WWW-Authenticate", "Basic");
						res.status = 401;
						return httplib::Server::HandlerResponse::Handled;
					}
					auto basic_auth_base64 = ohtoai::tool::string::split(
						ohtoai::tool::string::trimmed(req.get_header_value("Authorization")), " ").back();
					auto auth = ohtoai::tool::string::split(brynet::base::crypto::base64_decode(basic_auth_base64), ":");

					std::transform(auth.begin(), auth.end(), auth.begin(), ohtoai::tool::string::trimmed);
					if (auth.size() != 2)
					{
						spdlog::error("Error base64: {}", basic_auth_base64);
						res.set_header("WWW-Authenticate", "Basic");
						res.status = 401;
						return httplib::Server::HandlerResponse::Handled;
					}

					if (auth[0] != globalUserLogin.username || auth[1] != globalUserLogin.password)
					{
						res.set_header("WWW-Authenticate", "Basic");
						res.status = 401;
						return httplib::Server::HandlerResponse::Handled;
					}

					spdlog::info("Auth {}", auth[0]);
				}
			}

			return httplib::Server::HandlerResponse::Unhandled;
		});

	for (auto hook : configJ["hook"])
	{
		std::string name = hook["name"];
		std::string method = hook["method"];
		std::string path = pathPrefix + hook["path"].get<std::string>();
		std::string command{};
		std::string result_from = hook["result"]["from"];
		std::string result_type = hook["result"]["type"];
		std::string result_value{};

		if (hook.contains("command"))
		{
			command = hook["command"];
		}
		if (hook["result"].contains("value"))
		{
			result_value = hook["result"]["value"];
		}

		if (result_type.empty())
			result_type = "text/plain";

		ohtoai::tool::string::transform_to_upper(method);

		spdlog::info("Bind `{}` {} {} hook, with command `{}`", name, method, path, command);

		auto handler = [=, &server](const httplib::Request& req, httplib::Response& res)
		{
			spdlog::info("Hook `{}`", name);
			std::string command_result{};

			if (command.empty())
			{
				spdlog::info("No command given");
			}
			else
			{
				command_result = executeCommand(command);
				spdlog::info("Run command `{}`\n{}", command, command_result);
			}

			if (result_from == "command")
			{
				res.set_content(command_result, result_type.c_str());
				spdlog::info("Set content to `{}`", command_result);
			}
			else if (result_from == "constant")
			{
				res.set_content(result_value, result_type.c_str());
				spdlog::info("Set content to `{}`", result_value);
			}
			else
			{
				spdlog::error("Unknown result_from `{}`", result_from);
			}
		};
		if (method == "GET")
		{
			server.Get(path.c_str(), handler);
		}
		else if (method == "POST")
		{
			server.Post(path.c_str(), handler);
		}
		else if (method == "Delete")
		{
			server.Delete(path.c_str(), handler);
		}
		else if (method == "Put")
		{
			server.Put(path.c_str(), handler);
		}
		else if (method == "Options")
		{
			server.Options(path.c_str(), handler);
		}
		else if (method == "Patch")
		{
			server.Patch(path.c_str(), handler);
		}
		else
		{
			spdlog::error("Illegal method: {}", method);
		}
	}

	return server.listen_after_bind();
}