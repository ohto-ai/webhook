# add pthread support
find_package (Threads REQUIRED)
if (THREADS_HAVE_PTHREAD_ARG)
  target_compile_options (${PROJECT_NAME} PUBLIC "-pthread")
endif ()
if (CMAKE_THREAD_LIBS_INIT)
  target_link_libraries (${PROJECT_NAME} "${CMAKE_THREAD_LIBS_INIT}")
endif ()
