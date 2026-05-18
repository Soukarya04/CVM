cmake_minimum_required(VERSION 3.10)                # minimum cmake version required
project(cvmpp LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)              # using c++17 for file system support
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# compiler warnings
if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# release/debug
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# executable section
add_executable(cvm src/main.cpp
    src/lexer.cpp
    src/parser.cpp
    src/compiler.cpp
    src/vm.cpp)

target_include_directories(cvm PRIVATE src)

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ standard: ${CMAKE_CXX_STANDARD}")
