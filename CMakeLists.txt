cmake_minimum_required(VERSION 3.22)
project(columnar_engine)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDART_REQUIRED ON)

include(cmake/dependencies_package.cmake)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wshadow -Werror -Wpedantic")
enable_testing()
add_subdirectory(src)
add_subdirectory(tests)
