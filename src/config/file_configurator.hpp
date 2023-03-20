
#include <functional>
#include <thread>
#include <map>
#include <nlohmann/json.hpp>
#include <ghc/fs_std.hpp>

class FileConfigurator;
struct ConfigItemRef
{
    using reference_t = nlohmann::json::json_pointer;
    using callback_t = std::function<void(FileConfigurator &, const reference_t &)>;
    using callback_list_t = std::vector<callback_t>;
    reference_t ref;
    callback_list_t on_changed;
};

class FileConfigurator
{
public:
    FileConfigurator(const std::string &configPath) : configPath{configPath} {};
    ConfigItemRef &addConfigItem(const ConfigItemRef::reference_t &ref)
    {
        configItems.emplace(ref, ConfigItemRef{ref, {}});
        return configItems.at(ref);
    }

    void load()
    {
        lastWriteTime = fs::last_write_time(configPath);
        fs::ifstream ifs(configPath);
        if (!ifs.is_open())
        {
            throw std::runtime_error("Failed to open config file: " + configPath);
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
                for (auto &cb : configItems.at(ref).on_changed)
                {
                    cb(*this, ref);
                }
            }
        }
    }
    void save()
    {
        fs::ofstream ofs(configPath);
        if (!ofs.is_open())
        {
            throw std::runtime_error("Failed to open config file: " + configPath);
        }
        ofs << configJson.dump(4);
    }

    bool enterMonitorLoop()
    {
        if (monitorThread.joinable())
        {
            return false;
        }
        monitorLoopRunning = true;
        monitorThread = std::thread([this]
                                    {
            while (monitorLoopRunning)
            {
                std::this_thread::sleep_for(std::chrono::seconds(3));
                if (fs::last_write_time(configPath) > lastWriteTime)
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
    T get(const ConfigItemRef::reference_t &ref)
    {
        return configJson.value(ref, T());
    }

    const nlohmann::json & getJson() const
    {
        return configJson;
    }

private:
    fs::file_time_type lastWriteTime;
    std::string configPath;
    std::map<ConfigItemRef::reference_t, ConfigItemRef> configItems;
    std::thread monitorThread;
    nlohmann::json configJson;
    std::atomic<bool> monitorLoopRunning{false};
};
