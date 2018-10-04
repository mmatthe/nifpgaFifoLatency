cmake_minimum_required(VERSION 2.8)
set(CMAKE_CXX_COMPILER /usr/bin/x86_64-nilrt-linux-g++)
add_definitions(-std=gnu++0x)

project(NIFPGA-FifoLatency)

add_subdirectory(src)

