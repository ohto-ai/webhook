cmake_minimum_required(VERSION 3.14)

option(BUILD_TESTS "Build test suite" ON)
option(ENABLE_SANITIZERS "Enable Address and Undefined Behavior sanitizers" OFF)
option(ENABLE_CLANG_TIDY "Enable clang-tidy analysis" OFF)

project(webhook)

add_subdirectory(src)
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(test)
endif()
