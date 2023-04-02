#include "config/config_monitor.hpp"
#include <catch2/catch.hpp>
#include <spdlog/spdlog.h>

TEST_CASE( "FileConfigurator can read and save file", "[FileConfigurator]" ) {
    using nlohmann::literals::operator"" _json_pointer;
    const std::string test_config_file_name = "file_config_monitor.test.json";
    const std::string test_config_content = R"TEST({"a":1,"b":true,"c":"str"})TEST";
    const std::string test_config_diff_from_empty = R"TEST([{"op":"add","path":"/a","value":1},{"op":"add","path":"/b","value":true},{"op":"add","path":"/c","value":"str"}])TEST";
    const nlohmann::json test_config_json = nlohmann::json::parse(test_config_content);
    ohtoai::file::ConfigMonitor configurator;
    if (configurator.exists()) {
        ghc::filesystem::remove(configurator.path());
    }

    auto write_empty_config = [&](){
        ghc::filesystem::ofstream ofs(test_config_file_name);
        ofs << "{}";
        ofs.close();
    };

    auto write_test_config = [&](){
        ghc::filesystem::ofstream ofs(test_config_file_name);
        ofs << test_config_json.dump(4);
        ofs.close();
    };

    REQUIRE(!configurator.exists());

    SECTION("assign config"){
        configurator = test_config_json;

        REQUIRE(configurator.at("a") == test_config_json.at("a"));
        REQUIRE(configurator.at("b") == test_config_json.at("b"));
        REQUIRE(configurator.at("c") == test_config_json.at("c"));
    }

    SECTION("save config"){
        configurator.set_config_path(test_config_file_name);
        configurator = test_config_json;
        configurator.save();

        REQUIRE(configurator.exists());
    }

    SECTION("load config"){
        write_test_config();
        configurator.set_config_path(test_config_file_name);

        REQUIRE(configurator.exists());
        REQUIRE(configurator.at("a") == test_config_json.at("a"));
        REQUIRE(configurator.at("b") == test_config_json.at("b"));
        REQUIRE(configurator.at("c") == test_config_json.at("c"));
    }

    
    SECTION("monitor callback"){
        configurator = nlohmann::json::parse("{}");
        configurator.set_config_path(test_config_file_name);
        configurator.set_callback([&](ohtoai::file::ConfigMonitor &cm, const nlohmann::json &diff){
            spdlog::info("Line {} configurator\n{}\ndiff\n{}", __LINE__, configurator.dump(4), diff.dump(4));
            REQUIRE(cm.at("a") == test_config_json.at("a"));
            REQUIRE(cm.at("b") == test_config_json.at("b"));
            REQUIRE(cm.at("c") == test_config_json.at("c"));

            REQUIRE(diff.dump() == test_config_diff_from_empty);
        });
        write_empty_config();
        configurator.load();
        configurator.enterMonitorLoop();
        write_test_config();
        spdlog::info("Line {} configurator\n{}", __LINE__, configurator.dump(4));

        std::this_thread::sleep_for(std::chrono::milliseconds(4000));

        spdlog::info("Line {} configurator\n{}", __LINE__, configurator.dump(4));

        REQUIRE(configurator.exists());
        REQUIRE(configurator.at("a") == test_config_json.at("a"));
        REQUIRE(configurator.at("b") == test_config_json.at("b"));
        REQUIRE(configurator.at("c") == test_config_json.at("c"));

        configurator.exitMonitorLoop();
    }
}
