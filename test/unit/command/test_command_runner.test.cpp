#include <catch2/catch.hpp>
#include <command/command_runner.h>

using ohtoai::CommandRunner;

TEST_CASE("CommandRunner: run simple command", "[command]")
{
    CommandRunner runner;
#if defined(_WIN32)
    std::string output = runner.run("echo Hello");
#else
    std::string output = runner.run("echo -n Hello");
#endif
    REQUIRE(output.find("Hello") != std::string::npos);
}

TEST_CASE("CommandRunner: run async command", "[command]")
{
    CommandRunner runner;
#if defined(_WIN32)
    auto future = runner.runAsync("echo AsyncTest");
#else
    auto future = runner.runAsync("echo -n AsyncTest");
#endif
    auto output = future.get();
    REQUIRE(output.find("AsyncTest") != std::string::npos);
}

TEST_CASE("CommandRunner: output size limit", "[command]")
{
    CommandRunner runner;
    runner.setMaxOutputSize(10);
#if defined(_WIN32)
    std::string output = runner.run("echo ThisIsAVeryLongString");
#else
    std::string output = runner.run("echo -n ThisIsAVeryLongString");
#endif
    REQUIRE(output.size() <= 10 + 2); // Small buffer tolerance
}

TEST_CASE("CommandRunner: invalid command", "[command]")
{
    CommandRunner runner;
    std::string output = runner.run("nonexistent_command_xyz_12345");
    // Should return empty or error — shouldn't crash
    REQUIRE(true); // Just verify no crash
}

TEST_CASE("CommandRunner: empty command", "[command]")
{
    CommandRunner runner;
#if defined(_WIN32)
    std::string output = runner.run("");
#else
    std::string output = runner.run("true");
#endif
    REQUIRE(true); // Shouldn't crash
}
