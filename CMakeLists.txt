cmake_minimum_required(VERSION 3.25)

set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -march=native")

add_executable(fast_walk src/main.cpp)
