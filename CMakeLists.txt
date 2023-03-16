cmake_minimum_required(VERSION 3.10)
project(webhook)

set(CMAKE_CXX_STANDARD 14)

# Add include directories
include_directories(inc)

# Add source files
set(SOURCES
    src/main.cpp
    src/util/platform.cpp
)

# Get the current Git hash
execute_process(
  COMMAND git rev-parse --short HEAD
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  OUTPUT_VARIABLE GIT_COMMIT_HASH
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

# Get latest commit date
execute_process(
  COMMAND git log -1 --format=%cd --date=short
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  OUTPUT_VARIABLE GIT_COMMIT_DATE
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

# Get the latest Git tag
execute_process(
  COMMAND git describe --abbrev=0 --tags
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  OUTPUT_VARIABLE GIT_LATEST_TAG
  OUTPUT_STRIP_TRAILING_WHITESPACE
  ERROR_QUIET
)

# Check if the latest tag is in the current commit's history
execute_process(
  COMMAND git rev-list ${GIT_LATEST_TAG}..HEAD
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  RESULT_VARIABLE IS_LATEST_TAG_IN_HISTORY
  ERROR_QUIET
)

# Set BUILD_VERSION
if (NOT GIT_LATEST_TAG OR IS_LATEST_TAG_IN_HISTORY)
  set(BUILD_VERSION "${GIT_COMMIT_HASH}")
else()
  set(BUILD_VERSION "${GIT_LATEST_TAG}")
endif()

add_definitions(-DPROJECT_NAME="${PROJECT_NAME}")
add_definitions(-DBUILD_VERSION="${BUILD_VERSION}")
add_definitions(-DCOMMIT_HASH="${GIT_COMMIT_HASH}")
add_definitions(-DCOMMIT_DATE="${GIT_COMMIT_DATE}")

# Set the output directory for the executable
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Build the executable
add_executable(${PROJECT_NAME} ${SOURCES})

# Link header-only libraries
# target_link_libraries(webhook ${HEADER_ONLY_LIBRARIES})

target_link_libraries(${PROJECT_NAME} pthread ssl crypto)
