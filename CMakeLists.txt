cmake_minimum_required(VERSION 3.20)
project(pubsub LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(PUBSUB_SANITIZER "" CACHE STRING "Sanitizer to enable: address, thread, undefined, or address,undefined")

if(PUBSUB_SANITIZER)
  add_compile_options(-fsanitize=${PUBSUB_SANITIZER} -fno-omit-frame-pointer -g)
  add_link_options(-fsanitize=${PUBSUB_SANITIZER})
endif()

# header-only library for now
add_library(pubsub INTERFACE)
target_include_directories(pubsub INTERFACE include)

add_library(itch INTERFACE)
target_include_directories(itch INTERFACE include)

add_executable(basic examples/basic.cpp)
target_link_libraries(basic PRIVATE pubsub)
target_compile_options(basic PRIVATE -Wall -Wextra -Wpedantic -Wconversion)

option(PUBSUB_BUILD_TESTS "Build tests" ON)
if(PUBSUB_BUILD_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif()
