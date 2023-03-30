#include "config/file_configurator.hpp"
#include <catch2/catch.hpp>

TEST_CASE( "FileConfigurator can read and save file", "[FileConfigurator]" ) {
    using nlohmann::literals::operator"" _json_pointer;
    const std::string test_config_file_name = "FileConfigurator.test.json";
    const std::string test_config_content = R"TEST({"a":1,"b":true,"c":"str"})TEST";
    const nlohmann::json test_config_json = nlohmann::json::parse(test_config_content);
    FileConfigurator configurator(test_config_file_name);

    SECTION("assign config"){
        configurator.assign(test_config_json);

        REQUIRE(configurator.get<int>("/a"_json_pointer) == test_config_json["a"]);
        REQUIRE(configurator.get<bool>("/b"_json_pointer) == test_config_json["b"]);
        REQUIRE(configurator.get<std::string>("/c"_json_pointer) == test_config_json["c"]);
        REQUIRE(configurator.getJson() == test_config_json);
    }

    SECTION("save config"){
        configurator.assign(test_config_json);
        configurator.save();

        REQUIRE(configurator.exists());
    }

    SECTION("load config"){
        fs::ofstream ofs(test_config_file_name);
        ofs << test_config_json.dump(4);
        ofs.close();
        configurator.load();

        REQUIRE(configurator.exists());
        REQUIRE(configurator.get<int>("/a"_json_pointer) == test_config_json["a"]);
        REQUIRE(configurator.get<bool>("/b"_json_pointer) == test_config_json["b"]);
        REQUIRE(configurator.get<std::string>("/c"_json_pointer) == test_config_json["c"]);
        REQUIRE(configurator.getJson() == test_config_json);
    }
}
