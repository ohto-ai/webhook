cmake_minimum_required(VERSION 3.10)
project(webhook)

set(CMAKE_CXX_STANDARD 14)

# Find OpenSSL Library
if (UNIX)
    find_package (OpenSSL REQUIRED)
endif ()

# include directories
include_directories (inc)
if (OPENSSL_FOUND)
    include_directories (${OPENSSL_INCLUDE_DIR})
endif ()

# Add include directories
include_directories(inc)

# Add source files
set(SOURCES
    src/main.cpp
    src/util/platform.cpp
)

# Get latest commit date
execute_process(
  COMMAND git log -1 --format=%cd --date=short
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  OUTPUT_VARIABLE GIT_COMMIT_DATE
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

# Get the current Git hash
execute_process(
  COMMAND git rev-parse --short HEAD
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  OUTPUT_VARIABLE GIT_COMMIT_HASH
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

# Check if the latest tag is the current commit
execute_process(
    COMMAND git describe --tags --exact-match HEAD
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    RESULT_VARIABLE GIT_CHECK_COMMIT_IN_TAGS
    OUTPUT_VARIABLE GIT_LATEST_TAG
    ERROR_QUIET
    OUTPUT_STRIP_TRAILING_WHITESPACE
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

add_definitions(-DPROJECT_NAME="${PROJECT_NAME}")
add_definitions(-DBUILD_VERSION="${BUILD_VERSION}")
add_definitions(-DCOMMIT_HASH="${GIT_COMMIT_HASH}")
add_definitions(-DCOMMIT_DATE="${GIT_COMMIT_DATE}")
add_definitions(-DIS_DEV_VERSION=${IS_DEV_VERSION})

# Set the output directory for the executable
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Build the executable
add_executable(${PROJECT_NAME} ${SOURCES})

# Link header-only libraries
# target_link_libraries(webhook ${HEADER_ONLY_LIBRARIES})

if (UNIX)
    if (OPENSSL_FOUND)
        target_link_libraries (${PROJECT_NAME} ${OPENSSL_LIBRARIES})
    endif ()
    target_link_libraries (${PROJECT_NAME} pthread ssl crypto)
else()
    target_link_libraries (${PROJECT_NAME} ws2_32 wsock32) # For Windows
endif()
