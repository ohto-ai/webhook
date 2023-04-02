cmake_minimum_required(VERSION 3.10)
project(webhook CXX)

set(CMAKE_CXX_STANDARD 17)

include(${CMAKE_SOURCE_DIR}/cmake/add_git_version.cmake)

configure_file(
    ${PROJECT_SOURCE_DIR}/pre/version.hpp.in
    ${PROJECT_BINARY_DIR}/generated/inc/version.hpp
)

# if on windows, add define WIN32_LEAN_AND_MEAN and NOMINMAX
if (WIN32)
    add_definitions(-DWIN32_LEAN_AND_MEAN)
    add_definitions(-DNOMINMAX)
endif()

# Add include directories
include_directories(${CMAKE_SOURCE_DIR}/inc)
include_directories(${PROJECT_BINARY_DIR}/generated/inc)

# Add source files
set(SOURCES
        main.cpp
        webhook_manager.cpp
        util/platform.cpp
)

# Set the output directory for the executable
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/$<CONFIG>)

# Build the executable
add_executable(${PROJECT_NAME} ${SOURCES})

find_package(Threads REQUIRED)
find_package(OpenSSL REQUIRED)

target_link_libraries(${PROJECT_NAME} PUBLIC Threads::Threads)
target_link_libraries(${PROJECT_NAME} PUBLIC OpenSSL::SSL OpenSSL::Crypto)
