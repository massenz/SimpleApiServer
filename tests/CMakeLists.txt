# This file (c) 2016-2017 AlertAvert.com.  All rights reserved.

project(apiserver_test)
enable_testing()
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -fPIC")

# Conan Packaging support
include(../.conan/conanbuildinfo.cmake)
conan_basic_setup()

###
# Third Party libraries
set(THIRD_PARTY ${CMAKE_SOURCE_DIR}/third_party)
set(SIMPLE_HTTP ${THIRD_PARTY}/SimpleHttpRequest)

include_directories(${SIMPLE_HTTP}/include
        ${SIMPLE_HTTP}/include/uv
)
link_directories(${SIMPLE_HTTP}/lib)


find_library(GTEST gtest ${INSTALL_DIR}/lib)
if (${GTEST} STREQUAL GTEST-NOTFOUND)
    message(FATAL_ERROR "Could not locate a valid Google Test library installed.")
endif (${GTEST} STREQUAL GTEST-NOTFOUND)

set(UNIT_TESTS
        ${TESTS_DIR}/test_apiserver.cpp
)

# Add the build directory to the library search path
link_directories(${CMAKE_BINARY_DIR})

add_executable(unit_tests
        ${UNIT_TESTS}
        tests.h
        all_tests.cpp
)
target_link_libraries(unit_tests
        ${GTEST}
        apiserver
        http_parser
        uv
)

add_test(unit_tests ${CMAKE_CURRENT_BINARY_DIR}/unit_tests)
