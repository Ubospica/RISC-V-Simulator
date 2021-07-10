cmake_minimum_required(VERSION 3.16)
project(code)

add_compile_options(-Ofast)

set(CMAKE_CXX_STANDARD 17)

add_executable(old old/main.cpp)
add_executable(code src/main.cpp)