cmake_minimum_required(VERSION 3.16)
project(testpi)

set(CMAKE_CXX_STANDARD 20)

include_directories(libloragw/inc)


add_executable(testpi main.cpp lib/sx1301.cpp lib/sx1301.h)
target_link_libraries(testpi ${CMAKE_SOURCE_DIR}/libloragw/libloragw.a)
