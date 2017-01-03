cmake_minimum_required (VERSION 2.6)
project (RxKVO-CPP)

include_directories("${PROJECT_SOURCE_DIR}/../../lib3rd/rxcpp.git/Rx/v2/src")
include_directories("${PROJECT_SOURCE_DIR}/../../lib3rd/catch.git/include")
include_directories("${PROJECT_SOURCE_DIR}/../../src/cpp")

set (CMAKE_CXX_STANDARD 11)

set(RX_KVO_HEADERS "${PROJECT_SOURCE_DIR}/../../src/cpp/kvo_extension.hpp")

add_executable(RxKVO-test ${RX_KVO_HEADERS} test.cpp)
