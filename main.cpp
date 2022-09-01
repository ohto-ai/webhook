#include <cpp-httplib/httplib.h>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>
#include <fstream>
#include <algorithm>
#include <iterator>
#include <brynet/base/crypto/Base64.hpp>

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

namespace ohtoai
{
	namespace tool
	{
		namespace string
		{
			std::vector<std::string> split(const std::string& s, const std::string& delimiters)
			{
				std::vector<std::string> tokens;
				std::string::size_type lastPos = s.find_last_not_of(delimiters, 0);
				std::string::size_type pos = s.find_first_of(delimiters, lastPos);
				while (std::string::npos != pos || std::string::npos != lastPos)
				{
					tokens.push_back(s.substr(lastPos, pos - lastPos));
					lastPos = s.find_first_not_of(delimiters, pos);
					pos = s.find_first_of(delimiters, lastPos);
				}
				return tokens;
			}

			std::string& trim(std::string& s)
			{
				if (s.empty())
					return s;

				s.erase(0, s.find_first_not_of(" "));
				s.erase(s.find_last_not_of(" ") + 1);
				return s;
			}
		}
	}
}

struct UserLogin {
	std::string username;
	std::string password;
};

int main()
{
	spdlog::info("Application start.");

	std::ifstream ifs("hook.json");
	if (!ifs) {
		spdlog::error("Failed to open hook.json");
		return -1;
	}

	nlohmann::json j;
	try {
		ifs >> j;
	}
	catch (nlohmann::detail::exception e)
	{
		spdlog::error("{}", e.what());
		return -1;
	}

	ifs.close();

	httplib::Server server;

	UserLogin globalUserLogin{ j["auth"]["username"], j["auth"]["password"] };
	bool isAuthEnabled = j["auth"]["enabled"];
	

	server.bind_to_port(j["server"]["host"].get<std::string>().c_str(), j["server"]["port"]);

	server.set_pre_routing_handler([&server, &globalUserLogin, &isAuthEnabled](const httplib::Request& req, httplib::Response& res)
		{
			spdlog::info("Routing {} {} \nReceive {} bytes\n{}", req.method, req.path, req.body.size(), req.body);

			if (isAuthEnabled)
			{
				if (req.method == "GET")
				{
					if (!req.has_header("Authorization") || req.get_header_value("Authorization").empty())
					{
						res.set_header("WWW-Authenticate", R"(Basic realm="Secure Area")");
						res.status = 401;
						return httplib::Server::HandlerResponse::Handled;
					}
					auto basic_auth_base64 = ohtoai::tool::string::split(req.get_header_value("Authorization"), " ").back();
					auto basic_auth = brynet::base::crypto::base64_decode(basic_auth_base64);
					auto auth = ohtoai::tool::string::split(basic_auth, ":");
					if (auth.size() != 2)
					{
						spdlog::error("Error base64: {}", basic_auth);
						res.set_header("WWW-Authenticate", R"(Basic realm="Secure Area")");
						res.status = 401;
						return httplib::Server::HandlerResponse::Handled;
					}

					if (auth[0] != globalUserLogin.username || auth[1] != globalUserLogin.password)
					{
						res.set_header("WWW-Authenticate", R"(Basic realm="Secure Area")");
						res.status = 401;
						return httplib::Server::HandlerResponse::Handled;
					}

					spdlog::info("Auth {}", auth[0]);
				}
			}
			
			return httplib::Server::HandlerResponse::Unhandled;
		});


	for (auto hook : j["hook"])
	{
		std::string name = hook["name"];
		std::string method = hook["method"];
		std::string path = hook["path"];
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

		if(result_type.empty())
			result_type = "text/plain";

		std::transform(method.begin(), method.end(), method.begin(), [](char ch) ->char { return std::toupper(ch); });

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