# This file (c) 2016-2020 AlertAvert.com.  All rights reserved.

project(apiserver)
cmake_minimum_required(VERSION 3.16)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -fPIC")

# Conan Packaging support
include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

##
# Include common cmake utilities.
# See: https://bitbucket.org/marco/common-utils
#
include(${COMMON_UTILS_DIR}/commons.cmake OPTIONAL RESULT_VARIABLE COMMONS_FILE)

##
#
# Installation directory; this is both used to access
# required libraries (microhttpd, distutils) as well as
# to install the compiled artifacts.
#
# The INSTALL_DIR env variable must be set, unless -DINSTALL_DIR is used.
if(NOT DEFINED INSTALL_DIR)
    if(DEFINED ENV{INSTALL_DIR})
        set(INSTALL_DIR $ENV{INSTALL_DIR})
    else()
        message(FATAL_ERROR
                "INSTALL_DIR should be defined as an environment variable, or via -D notation")
    endif()
endif()

if(NOT DEFINED LIBMICROHTTPD_INSTALL)
    set(LIBMICROHTTPD_INSTALL ${INSTALL_DIR}/libmicrohttpd)
endif()

##
# Release version management
#
set(RELEASE_MAJOR 0)
set(RELEASE_MINOR 3)
set(RELEASE_PATCH 1)

set(RELEASE "${RELEASE_MAJOR}.${RELEASE_MINOR}.${RELEASE_PATCH}")
if(DEFINED COMMONS_FILE)
    get_build_id(BUILD_ID)
    set(RELEASE_STRING "${RELEASE}-${BUILD_ID}")
else()
    set(RELEASE_STRING "${RELEASE}")
endif()
message("Building ${PROJECT_NAME} ${RELEASE_STRING} (${CMAKE_BUILD_TYPE})")


set(SOURCE_DIR ${PROJECT_SOURCE_DIR}/src)
set(INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include)
set(TESTS_DIR ${PROJECT_SOURCE_DIR}/tests)
set(PROTO_DIR ${PROJECT_SOURCE_DIR}/proto)
set(PROJECT_BINARY_DIR ${PROJECT_SOURCE_DIR}/build)

##
# Depending on the build type (Debug/Release) Google Logging
# generates a differently-named library.
# Both the value of CMAKE_BUILD_TYPE and the `build_type` setting
# in `~/.conan/profiles/default` need to match.
#
if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    set(GLOG glogd)
else()
    set(GLOG glog)
endif()


# Configure a header file to pass settings into source code.
configure_file(
    ${SOURCE_DIR}/config.h.in
    ${PROJECT_BINARY_DIR}/version.h
)


include_directories(
        ${INCLUDE_DIR}
        ${PROJECT_BINARY_DIR}
        ${INSTALL_DIR}/include
        ${LIBMICROHTTPD_INSTALL}/include
)

link_directories(
        ${INSTALL_DIR}/lib
        ${LIBMICROHTTPD_INSTALL}/lib
)

set(SOURCES
        ${SOURCE_DIR}/api/rest/ApiServer.cpp
)

set(LIBS
        ${GLOG}
        distutils
        microhttpd
        pthread
)

##
# Utility program to print version string
##
add_executable(get_version ${SOURCE_DIR}/get_version.cpp)

##
# libapiserver.so
#
# Shared (dynamic) library for the APIs.
#
add_library(${PROJECT_NAME} SHARED
        ${SOURCES}
)
target_link_libraries(${PROJECT_NAME} ${LIBS})


##
# Installation
#
message("Installing shared library lib${PROJECT_NAME}.so to ${INSTALL_DIR}/lib")
install(TARGETS ${PROJECT_NAME} DESTINATION ${INSTALL_DIR}/lib)
config_install(${INSTALL_DIR} INSTALL_RESULT)


# Unit tests build is defined in the tests/ folder
add_subdirectory(${TESTS_DIR})


# Examples will use the latest build for the shared libraries
link_directories(
        ${CMAKE_BINARY_DIR}/lib
        ${INSTALL_DIR}/lib
)

# Demo server to either listen to incoming pings, or send.
#
include_directories(${INSTALL_DIR}/include)
add_executable(server_demo
        ${SOURCES}
        ${SOURCE_DIR}/examples/server_demo.cpp
)
target_link_libraries(server_demo
        ${LIBS}
)
