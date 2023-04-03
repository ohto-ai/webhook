set(GIT_COMMIT_HASH "unknown")
set(GIT_COMMIT_DATE "unknown")
set(BUILD_VERSION "unknown")
set(IS_DEV_VERSION true)

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
