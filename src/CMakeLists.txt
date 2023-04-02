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

# Add file_monitor
set(file_monitor_PLATFORM ${${PROJECT_NAME}_PLATFORM})
add_subdirectory(3rd/ltjax/file_monitor)
list(APPEND ThirdParty file_monitor)

# Build the executable
add_executable(${PROJECT_NAME} ${SOURCES})

find_package(Threads REQUIRED)
find_package(OpenSSL REQUIRED)

target_link_libraries(${PROJECT_NAME} PUBLIC Threads::Threads)
target_link_libraries(${PROJECT_NAME} PUBLIC OpenSSL::SSL OpenSSL::Crypto)

# link ThirdParty list
foreach(ThirdPartyLib ${ThirdParty})
    target_link_libraries(${PROJECT_NAME} PRIVATE ${ThirdPartyLib})
endforeach()
