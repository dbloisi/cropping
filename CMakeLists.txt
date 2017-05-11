project(cropping)

cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

find_package(OpenCV REQUIRED)

add_executable (${PROJECT_NAME} main imagemanager)
target_link_libraries (${PROJECT_NAME} ${OpenCV_LIBS})
