cmake_minimum_required(VERSION 3.10)
project(ParallelGrep)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include_directories(${PROJECT_SOURCE_DIR})

add_library(thread_pool_lib
    lib/thread_pool/blocking_queue.cpp
    lib/thread_pool/thread_pool.cpp
)
