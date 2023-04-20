cmake_minimum_required(VERSION 3.10)

include(../cmake/add_git_version.cmake)
project(ohtoai-webhook LANGUAGES CXX VERSION ${BUILD_VERSION_MAJOR}.${BUILD_VERSION_MINOR}.${BUILD_VERSION_PATCH})
set(CMAKE_CXX_STANDARD 17)
set(AUTHOR "OhtoAi")
set(EMAIL "zhu.thatboy@outlook.com")
set(DESCRIPTION "A webhook server")
if(WIN32)
    set(BINARY_SUFFIX ".exe")
    set(RELEASE_DIR "Release/")
elseif(APPLE)
    set(BINARY_SUFFIX ".img")
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

# Check and add bigobj option
if(CMAKE_COMPILER_IS_GNUCXX)
  include(CheckCXXCompilerFlag)
  check_cxx_compiler_flag("-Wa,-mbig-obj" GNU_BIG_OBJ_FLAG_ENABLE)
endif()
target_compile_options(${PROJECT_NAME}
	PRIVATE
	$<$<CXX_COMPILER_ID:MSVC>:/bigobj>
	$<$<AND:$<CXX_COMPILER_ID:GNU>,$<BOOL:${GNU_BIG_OBJ_FLAG_ENABLE}>>:-Wa,-mbig-obj>) 

# Add include directories
target_include_directories(${PROJECT_NAME} PRIVATE ../inc)
target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_BINARY_DIR}/generated/inc)

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

if (UNIX AND NOT APPLE)
    include(../cmake/ubuntu_package.cmake)
endif()
