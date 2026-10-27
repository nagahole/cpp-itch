cmake_minimum_required(VERSION 3.20)
project(pubsub LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# header-only library for now
add_library(pubsub INTERFACE)
target_include_directories(pubsub INTERFACE include)

add_executable(basic examples/basic.cpp)
target_link_libraries(basic PRIVATE pubsub)
target_compile_options(basic PRIVATE -Wall -Wextra -Wpedantic)

option(PUBSUB_BUILD_TESTS "Build tests" ON)
if(PUBSUB_BUILD_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif()
