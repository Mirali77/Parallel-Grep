cmake_minimum_required(VERSION 3.10)
project(ParallelGrep)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include(FetchContent)

FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        v1.17.0
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

include_directories(${PROJECT_SOURCE_DIR})

add_library(thread_pool_lib
    lib/thread_pool/blocking_queue.cpp
    lib/thread_pool/thread_pool.cpp
)

add_library(utils
    utils/test_helpers.cpp
)

add_executable(
    thread_pool_lib_tests
    lib/thread_pool/blocking_queue_tests.cpp
    lib/thread_pool/thread_pool_tests.cpp
)
target_link_libraries(thread_pool_lib_tests PRIVATE thread_pool_lib gtest gtest_main)