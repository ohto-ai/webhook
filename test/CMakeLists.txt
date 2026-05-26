cmake_minimum_required(VERSION 3.14)
project(webhook-tests)

set(CMAKE_CXX_STANDARD 17)

include_directories(${CMAKE_SOURCE_DIR}/inc)
include_directories(${CMAKE_SOURCE_DIR}/src)
include_directories(${CMAKE_SOURCE_DIR}/src/generated/inc)

find_package(Threads REQUIRED)
find_package(OpenSSL QUIET)

add_definitions(-DCATCH_CONFIG_MAIN)

function(add_webhook_test TEST_NAME TEST_FILE)
    add_executable(${TEST_NAME} ${TEST_FILE})
    target_link_libraries(${TEST_NAME} PRIVATE Threads::Threads)
    if(UNIX)
        target_link_libraries(${TEST_NAME} PRIVATE OpenSSL::SSL OpenSSL::Crypto)
    elseif(WIN32)
        target_link_libraries(${TEST_NAME} PRIVATE ws2_32)
    endif()
    add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
endfunction()

# Unit tests
add_webhook_test(test_config_model     unit/config/test_config_model.test.cpp)
add_webhook_test(test_basic_auth       unit/auth/test_basic_auth.test.cpp)
add_webhook_test(test_hmac_auth        unit/auth/test_hmac_auth.test.cpp)
add_webhook_test(test_command_runner   unit/command/test_command_runner.test.cpp)
add_webhook_test(test_rate_limiter     unit/server/test_rate_limiter.test.cpp)
add_webhook_test(test_request_validator unit/server/test_request_validator.test.cpp)
add_webhook_test(test_cors_middleware  unit/server/test_cors_middleware.test.cpp)
add_webhook_test(test_template_engine  unit/webhook/test_template_engine.test.cpp)
add_webhook_test(test_metrics_collector unit/metrics/test_metrics_collector.test.cpp)

# Integration tests
add_webhook_test(test_http_endpoints   integration/test_http_endpoints.test.cpp)
add_webhook_test(test_auth_integration integration/test_auth_integration.test.cpp)
