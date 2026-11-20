cmake_minimum_required(VERSION 3.16)
project(careroute LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

add_compile_options(-Wall -Wextra)

# The solver as a small library, reused by the runner and the tests.
add_library(careroute_core
  src/careroute/model.cpp
  src/careroute/solution.cpp
  src/careroute/construct.cpp
  src/careroute/localsearch.cpp
  src/careroute/lns.cpp
  src/careroute/io.cpp)
target_include_directories(careroute_core PUBLIC src)

# The command-line runner.
add_executable(careroute src/main.cpp)
target_link_libraries(careroute PRIVATE careroute_core)

# The tests.
enable_testing()
add_executable(careroute_tests tests/test_careroute.cpp)
target_include_directories(careroute_tests PRIVATE tests)
target_link_libraries(careroute_tests PRIVATE careroute_core)
add_test(NAME careroute_tests COMMAND careroute_tests)
