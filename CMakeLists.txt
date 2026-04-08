cmake_minimum_required(VERSION 3.22)
project(columnar_engine)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDART_REQUIRED ON)

include(cmake/dependencies_package.cmake)
include_directories(${CMAKE_SOURCE_DIR})


set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wshadow -Werror -Wpedantic")
find_package(glog REQUIRED)
find_package(GTest REQUIRED)
list(APPEND CMAKE_PREFIX_PATH "/opt/homebrew/opt/boost")
find_package(Boost REQUIRED)

enable_testing()
add_subdirectory(src)
add_subdirectory(tests)
add_subdirectory(exec)
