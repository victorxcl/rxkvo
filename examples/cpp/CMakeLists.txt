cmake_minimum_required (VERSION 2.6)
project (RxKVO-CPP)

include_directories("${PROJECT_SOURCE_DIR}/../../lib3rd/rxcpp.git/Rx/v2/src")
include_directories("${PROJECT_SOURCE_DIR}/../../lib3rd/catch.git/include")

set (CMAKE_CXX_STANDARD 11)

add_executable(RxKVO-test test.cpp)
