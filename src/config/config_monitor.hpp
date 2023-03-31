#pragma once

#include <map>
#include <mutex>
#include <atomic>
#include <thread>
#include <functional>
#include <nlohmann/json.hpp>
#include <ghc/filesystem.hpp>

namespace ohtoai::file
{
    class ConfigMonitor;
    using pointer_t = nlohmann::json::json_pointer;
    using json_t = nlohmann::json;

    using MonitorNotifyCallback = std::function<void(ConfigMonitor &, const json_t &)>;

    class ConfigMonitor : public nlohmann::json, private std::mutex
    {

    public:
        ConfigMonitor() {};
        ConfigMonitor(const ghc::filesystem::path &config_path) { set_config_path(config_path);};
        ConfigMonitor(const std::string &config_path) : ConfigMonitor{ghc::filesystem::path{config_path}} {};

        using nlohmann::json::operator=;

        void set_config_path(const ghc::filesystem::path &cp)
        {
            configPath = cp;
            load();
        }

        void set_callback(MonitorNotifyCallback cb)
        {
            callback = cb;
        }

        bool exists() const
        {
            return ghc::filesystem::exists(configPath);
        }

        decltype(auto) filename() const
        {
            return configPath.filename().string();
        }

        decltype(auto) path() const
        {
            return configPath.string();
        }

        void load()
        {
            std::lock_guard<std::mutex> lock(*this);
            // check if file exists
            if(!exists())
            {
                nlohmann::json _{};
                swap(_);
                if(callback)
                    callback(*this, nlohmann::json::diff(_, *this));
                return;
            }

            lastWriteTime = ghc::filesystem::last_write_time(configPath);
            ghc::filesystem::ifstream ifs(configPath);
            nlohmann::json _{};
            if (ifs.is_open())
            {
                try
                {
                    ifs >> _;
                }
                catch(const std::exception& e)
                {
                    _ = {};
                }
                ifs.close();
            }

            swap(_);
            auto df = nlohmann::json::diff(_, *this);
            if(!df.empty() && callback)
                callback(*this, df);
        }

        void save()
        {
            std::lock_guard<std::mutex> lock(*this);
            ghc::filesystem::ofstream ofs(configPath);
            if (ofs.is_open())
            {
                ofs << dump(4);
                ofs.close();
            }
            lastWriteTime = ghc::filesystem::last_write_time(configPath);
        }

        bool enterMonitorLoop(int sec = 3)
        {
            if (monitorThread.joinable())
            {
                return false;
            }
            monitorLoopRunning = true;
            monitorThread = std::thread([this, sec]
                                        {
            while (monitorLoopRunning)
            {
                std::this_thread::sleep_for(std::chrono::seconds(sec));
                if (ghc::filesystem::last_write_time(configPath) != lastWriteTime)
                {
                    load();
                }
                std::this_thread::yield();
            } });
            return true;
        }

        void exitMonitorLoop()
        {
            monitorLoopRunning = false;
            if (monitorThread.joinable())
            {
                monitorThread.join();
            }
        }

        ~ConfigMonitor()
        {
            exitMonitorLoop();
        }

    private:
        MonitorNotifyCallback callback;
        ghc::filesystem::file_time_type lastWriteTime;
        ghc::filesystem::path configPath;
        std::thread monitorThread;
        std::atomic<bool> monitorLoopRunning{false};
    };
}
