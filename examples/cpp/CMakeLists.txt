cmake_minimum_required (VERSION 2.6)
project (RxKVO-CPP)

IF(WIN32)
    ADD_DEFINITIONS(/bigobj)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /bigobj")
    SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /bigobj")
    SET(CMAKE_CXX_FLAGS_MINSIZEREL "${CMAKE_CXX_FLAGS_MINSIZEREL} /bigobj")
    SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /bigobj")
    SET(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} /bigobj")
ENDIF(WIN32)

include_directories("${PROJECT_SOURCE_DIR}/../../lib3rd/rxcpp.git/Rx/v2/src")
include_directories("${PROJECT_SOURCE_DIR}/../../lib3rd/catch.git/single_include")
include_directories("${PROJECT_SOURCE_DIR}/../../src/cpp")

set (CMAKE_CXX_STANDARD 14)

set(RX_KVO_HEADERS "${PROJECT_SOURCE_DIR}/../../src/cpp/kvo_extension.hpp")

add_executable(RxKVO-test ${RX_KVO_HEADERS} 
    test_basic.cpp 
    test_basic_initialize.cpp
    test_basic_operator.cpp 
	test_course_introduction.cpp
	test_course_login_register.cpp
	test_course_network.cpp
    test_library.cpp
    test_mail.cpp
    test_plan.cpp
    test_school.cpp
    test_subject_will_did.cpp
    test_wechat.cpp
)
