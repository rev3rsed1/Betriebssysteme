cmake_minimum_required(VERSION 3.12)
project(prak4)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS -pthread)

add_executable(prak4 main.cpp BunQueue.h ClientQueue.h)