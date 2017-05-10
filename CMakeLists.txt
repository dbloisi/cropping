project(crop)

cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

set(CMAKE_CXX_FLAGS "-std=c++11")

find_package(OpenCV REQUIRED)

include_directories(${PROJECT_BINARY_DIR})

add_executable (crop main.cpp imagemanager)
target_link_libraries (crop ${OpenCV_LIBS})
