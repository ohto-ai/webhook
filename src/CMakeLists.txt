cmake_minimum_required(VERSION 3.10)
project(webhook CXX)

set(CMAKE_CXX_STANDARD 17)

# Find OpenSSL Library
if (APPLE)
    set(OPENSSL_ROOT_DIR "/usr/local/opt/openssl" CACHE STRING "openssl root" FORCE)
    find_package (OpenSSL REQUIRED)
endif ()

set(GIT_COMMIT_HASH "unknown")
set(GIT_COMMIT_DATE "unknown")
set(BUILD_VERSION "unknown")
set(IS_DEV_VERSION true)

# find Git and if available set GIT_HASH variable
find_package(Git QUIET)
if(GIT_FOUND)
    # Get the current Git hash
    execute_process(
        COMMAND ${GIT_EXECUTABLE} log -1 --pretty=format:%h
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
        OUTPUT_VARIABLE GIT_COMMIT_HASH
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
    )
    # Get latest commit date
    execute_process(
        COMMAND ${GIT_EXECUTABLE} log -1 --format=%cd --date=short
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
        OUTPUT_VARIABLE GIT_COMMIT_DATE
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
    )

    # Check if the latest tag is the current commit
    execute_process(
        COMMAND ${GIT_EXECUTABLE} describe --tags --exact-match HEAD
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
        RESULT_VARIABLE GIT_CHECK_COMMIT_IN_TAGS
        OUTPUT_VARIABLE GIT_LATEST_TAG
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
    )

    if (${GIT_CHECK_COMMIT_IN_TAGS} EQUAL 0)
        # If the latest tag is the current commit, set BUILD_VERSION to the tag name
        set(BUILD_VERSION ${GIT_LATEST_TAG})
        set(IS_DEV_VERSION false)
        message(STATUS "Build version: ${BUILD_VERSION}(${GIT_COMMIT_HASH})")
    else()
        # If the latest tag is not the current commit, set BUILD_VERSION to the commit hash
        set(BUILD_VERSION ${GIT_COMMIT_HASH})
        set(IS_DEV_VERSION true)
        message(STATUS "Build version: ${BUILD_VERSION}")
    endif()
endif()

configure_file(
    ${PROJECT_SOURCE_DIR}/pre/version.hpp.in
    ${PROJECT_BINARY_DIR}/generated/inc/version.hpp
)

# if on windows, add define WIN32_LEAN_AND_MEAN and NOMINMAX
if (WIN32)
    add_definitions(-DWIN32_LEAN_AND_MEAN)
    add_definitions(-DNOMINMAX)
endif()

# Add openssl include directories
if (APPLE AND OPENSSL_FOUND)
    include_directories (${OPENSSL_INCLUDE_DIR})
endif ()

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

# Link header-only libraries
# target_link_libraries(webhook ${HEADER_ONLY_LIBRARIES})
if (UNIX)
    if (APPLE AND OPENSSL_FOUND)
        message(STATUS "OpenSSL found: ${OPENSSL_VERSION}")
        target_link_libraries (${PROJECT_NAME} ${OPENSSL_LIBRARIES})
    else()
        target_link_libraries (${PROJECT_NAME} pthread ssl crypto)
    endif ()
else()
    target_link_libraries (${PROJECT_NAME} ws2_32 wsock32) # For Windows
endif()
