#include <cpp-httplib/httplib.h>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>
#include <fstream>

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
	
	server.bind_to_port(j["server"]["host"].get<std::string>().c_str(), j["server"]["port"]);

	server.set_pre_routing_handler([&server](const httplib::Request& req, httplib::Response& res)
		{
			spdlog::info("Routing {} {} \n{} bytes\n{}", req.method, req.path, req.body.size(), req.body);
			return httplib::Server::HandlerResponse::Unhandled;
		});

	
	for (auto hook : j["hook"])
	{
		auto name = hook["name"].get<std::string>();
		auto method = hook["method"].get<std::string>();
		auto path = hook["path"].get<std::string>();
		auto command = hook["command"].get<std::string>();
		std::transform(method.begin(), method.end(), method.begin(), [](char ch) ->char { return std::toupper(ch); });

		spdlog::info("Bind `{}` {} {} hook, with command `{}`", name, method, path, command);
		
		auto handler = [=, &server](const httplib::Request& req, httplib::Response& res)
		{
			spdlog::info("Hook `{}`", name);
			auto ret = system(command.c_str());
			spdlog::info("Run command `{}` return {}", command, ret);
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