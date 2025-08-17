cmake_minimum_required(VERSION 3.21)

project(kvs CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(BUILD_TESTS "Enable building tests" ON)
message(STATUS "BUILD_TESTS: ${BUILD_TESTS}")
option(KVS_ENABLE_LOGS "Enable logs" OFF)
message(STATUS "KVS_ENABLE_LOGS: ${KVS_ENABLE_LOGS}")

if (BUILD_TESTS)
    find_package(GTest REQUIRED)
    enable_testing()
    include(GoogleTest)
endif()

add_subdirectory(src)