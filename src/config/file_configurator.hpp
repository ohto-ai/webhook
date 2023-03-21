#pragma once

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#endif

#include <functional>
#include <thread>
#include <map>
#include <nlohmann/json.hpp>
#include <ghc/fs_std.hpp>

class FileConfigurator;
using ConfigReference = nlohmann::json::json_pointer;
using ConfigCallback = std::function<void(FileConfigurator &, const ConfigReference &, const nlohmann::json& diff)>;

struct ConfigCallbackSet: public std::vector<ConfigCallback>
{
    ConfigCallbackSet& += (const ConfigCallback&cb){
        push_back(cb);
        return *this;
    }

    void operator ()(FileConfigurator &fc, const ConfigReference &ref, const nlohmann::json& diff){
        for(auto& cb: *this)
        {
            if(cb)
            {
                cb(fc, ref, diff);
            }
        }
    }
};

struct ConfigItemRef
{
    ConfigCallbackSet on_changed;
};

class FileConfigurator
{
public:
    FileConfigurator(const fs::path &configPath) : configPath{configPath} {};
    FileConfigurator(const std::string &configPath) : configPath{configPath} {};
    ConfigItemRef &addConfigItem(const ConfigReference &ref)
    {
        return *configItems.emplace(ref, {}}).first;
    }

    void load()
    {
        lastWriteTime = fs::last_write_time(configPath);
        fs::ifstream ifs(configPath);
        if (!ifs.is_open())
        {
            throw std::runtime_error("Failed to open config file: " + configPath.string());
        }
        nlohmann::json configJ = configJson;
        ifs >> configJson;
        ifs.close();

        auto df = nlohmann::json::diff(configJ, configJson);

        for (auto &df_obj : df)
        {
            auto &path = df_obj.at("path");
            auto ref = nlohmann::json::json_pointer(path);
            auto &op = df_obj.at("op");

            if (configItems.find(ref) != configItems.end())
            {
                configItems.at(ref).on_changed(*this, ref, df_obj);
            }
        }
    }

    void assign(const nlohmann::json & j)
    {
        configJson = j;
    }

    void save()
    {
        fs::ofstream ofs(configPath);
        if (!ofs.is_open())
        {
            throw std::runtime_error("Failed to open config file: " + configPath.string());
        }
        ofs << configJson.dump(4);
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
                if (fs::last_write_time(configPath) != lastWriteTime)
                {
                    load();
                }
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

    template <typename T>
    T get(const ConfigReference &ref)
    {
        return configJson.value(ref, T());
    }

    const nlohmann::json & getJson() const
    {
        return configJson;
    }

    ~FileConfigurator()
    {
        exitMonitorLoop();
    }

private:
    fs::file_time_type lastWriteTime;
    fs::path configPath;
    std::map<ConfigReference, ConfigItemRef> configItems;
    std::thread monitorThread;
    nlohmann::json configJson;
    std::atomic<bool> monitorLoopRunning{false};
};
