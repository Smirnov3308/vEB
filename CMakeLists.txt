cmake_minimum_required(VERSION 3.9)
project(vEB_v2)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -fpermissive -fgnu-tm")

add_executable(vEB_v2 main.cpp vEB.h)