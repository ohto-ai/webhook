cmake_minimum_required(VERSION 3.10)
project(webhook CXX)

set(CMAKE_CXX_STANDARD 17)

# Set the output directory for the executable
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/$<CONFIG>)

set(${PROJECT_NAME}_PLATFORM Undefined)
if(WIN32)
    set(${PROJECT_NAME}_PLATFORM PLATFORM_WIN32)
elseif(APPLE)
    set(${PROJECT_NAME}_PLATFORM PLATFORM_MACOS)
elseif(UNIX AND NOT APPLE)
    set(${PROJECT_NAME}_PLATFORM PLATFORM_LINUX)
endif()

include(../cmake/add_git_version.cmake)

configure_file(
    ${PROJECT_SOURCE_DIR}/pre/version.hpp.in
    ${PROJECT_BINARY_DIR}/generated/inc/version.hpp
)

# if on windows, add define WIN32_LEAN_AND_MEAN and NOMINMAX
if (WIN32)
    add_definitions(-DWIN32_LEAN_AND_MEAN)
    add_definitions(-DNOMINMAX)
endif()

# Add source files
set(SOURCES
    main.cpp
    webhook_manager.cpp
    util/platform.cpp
)

# Build the executable
add_executable(${PROJECT_NAME} ${SOURCES})

# Add include directories
target_include_directories(${PROJECT_NAME} PRIVATE ../inc)
target_include_directories(${PROJECT_NAME} PRIVATE 3rd)
target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_BINARY_DIR}/generated/inc)

# Add file_monitor
set(file_monitor_PLATFORM ${${PROJECT_NAME}_PLATFORM})
add_subdirectory(3rd/ltjax/file_monitor)
list(APPEND ThirdParty file_monitor)

# Link ThirdParty list
target_link_libraries(${PROJECT_NAME} PRIVATE ${ThirdParty})

# Link thread and ssl lib
if (APPLE)
    set(OPENSSL_ROOT_DIR "/usr/local/opt/openssl" CACHE STRING "openssl root" FORCE)
endif()
find_package(Threads REQUIRED)
find_package(OpenSSL QUIET)
target_link_libraries(${PROJECT_NAME} PRIVATE Threads::Threads)
if (UNIX)
    target_link_libraries(${PROJECT_NAME} PRIVATE OpenSSL::SSL OpenSSL::Crypto)
elseif(WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE ws2_32)
endif()
