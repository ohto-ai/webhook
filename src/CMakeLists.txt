cmake_minimum_required(VERSION 3.14)

include(../cmake/add_git_version.cmake)
include(../cmake/sanitizers.cmake)
project(ohtoai-webhook LANGUAGES CXX VERSION ${BUILD_VERSION_MAJOR}.${BUILD_VERSION_MINOR}.${BUILD_VERSION_PATCH})
set(CMAKE_CXX_STANDARD 17)
set(AUTHOR "OhtoAi")
set(EMAIL "zhu.thatboy@outlook.com")
set(DESCRIPTION "A webhook server")
if(WIN32)
    set(BINARY_SUFFIX ".exe")
    set(RELEASE_DIR "Release/")
elseif(APPLE)
    set(BINARY_SUFFIX "")
    set(RELEASE_DIR "")
else()
    set(BINARY_SUFFIX "")
    set(RELEASE_DIR "")
endif()

configure_file(
    ${PROJECT_SOURCE_DIR}/pre/version.hpp.in
    ${PROJECT_BINARY_DIR}/generated/inc/version.hpp
)
configure_file(
    ../scripts/env.sh.in
    ${CMAKE_SOURCE_DIR}/scripts/env.sh
)

# Windows-specific defines
if (WIN32)
    add_definitions(-DWIN32_LEAN_AND_MEAN)
    add_definitions(-DNOMINMAX)
endif()

# Library sources (everything except main.cpp)
set(LIB_SOURCES
    application.cpp
    server_manager.cpp

    config/config_manager.cpp

    auth/auth_manager.cpp
    auth/basic_auth.cpp
    auth/hmac_auth.cpp

    webhook/template_engine.cpp
    webhook/hook_handler.cpp

    server/http_server.cpp
    server/rate_limiter.cpp
    server/request_validator.cpp
    server/cors_middleware.cpp
    server/health_endpoint.cpp

    command/command_runner.cpp

    logging/logging_manager.cpp

    metrics/metrics_collector.cpp

    util/platform.cpp
)

# Static library for sharing between executable and tests
add_library(${PROJECT_NAME}-lib STATIC ${LIB_SOURCES})
add_library(ohtoai::webhook ALIAS ${PROJECT_NAME}-lib)

target_include_directories(${PROJECT_NAME}-lib
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
    PUBLIC ../inc
    PUBLIC ${PROJECT_BINARY_DIR}/generated/inc
)

# Check and add bigobj option
if(CMAKE_COMPILER_IS_GNUCXX)
  include(CheckCXXCompilerFlag)
  check_cxx_compiler_flag("-Wa,-mbig-obj" GNU_BIG_OBJ_FLAG_ENABLE)
endif()
target_compile_options(${PROJECT_NAME}-lib
    PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/bigobj /FS>
    $<$<AND:$<CXX_COMPILER_ID:GNU>,$<BOOL:${GNU_BIG_OBJ_FLAG_ENABLE}>>:-Wa,-mbig-obj>)

# Link thread and ssl lib
if (APPLE)
    set(OPENSSL_ROOT_DIR "/usr/local/opt/openssl" CACHE STRING "openssl root" FORCE)
endif()
find_package(Threads REQUIRED)
find_package(OpenSSL QUIET)
target_link_libraries(${PROJECT_NAME}-lib PUBLIC Threads::Threads)
if (UNIX)
    target_link_libraries(${PROJECT_NAME}-lib PUBLIC OpenSSL::SSL OpenSSL::Crypto)
elseif(WIN32)
    target_link_libraries(${PROJECT_NAME}-lib PUBLIC ws2_32)
endif()

# Main executable
add_executable(${PROJECT_NAME} main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE ${PROJECT_NAME}-lib)

if (UNIX AND NOT APPLE)
    include(../cmake/ubuntu_package.cmake)
endif()
