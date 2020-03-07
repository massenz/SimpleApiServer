# This file (c) 2016-2017 AlertAvert.com.  All rights reserved.

project(apiserver_test)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -fPIC")

enable_testing()

# Conan Packaging support
include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

find_library(GTEST gtest ${INSTALL_DIR}/lib)
if (${GTEST} STREQUAL GTEST-NOTFOUND)
    message(FATAL_ERROR "Could not locate a valid Google Test library installed.")
endif (${GTEST} STREQUAL GTEST-NOTFOUND)


include_directories(
        ${INSTALL_DIR}/include
)

set(UNIT_TESTS
        ${TESTS_DIR}/test_apiserver.cpp
        ${TESTS_DIR}/test_request_response.cpp
)

# Add the build directory to the library search path
link_directories(
        ${CMAKE_BINARY_DIR}/lib
        ${INSTALL_DIR}/lib
)

add_executable(unit_tests
        ${SOURCES}
        ${UNIT_TESTS}
        tests.h
        all_tests.cpp
)
target_link_libraries(unit_tests
        ${GTEST}
        glog
        http_parser
        microhttpd
        pthread
        uv
)

add_test(unit_tests ${CMAKE_CURRENT_BINARY_DIR}/unit_tests)
