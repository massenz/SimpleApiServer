# This file (c) 2016-2017 AlertAvert.com.  All rights reserved.

project(apiserver)
cmake_minimum_required(VERSION 3.4)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -fPIC")


# Include common cmake utilities:
include(${COMMON_UTILS_DIR}/commons.cmake OPTIONAL RESULT_VARIABLE COMMONS_FILE)

### Setting up common configuration variables

# Version number
set(RELEASE_MAJOR 0)
set(RELEASE_MINOR 1)
set(RELEASE_PATCH 0)

IF (${COMMONS_FILE} STREQUAL NOTFOUND)
    message(WARNING "Not using CMake utilities")
    set(BUILD_ID 0)
ELSE (${COMMONS_FILE} STREQUAL NOTFOUND)
    get_build_id(BUILD_ID)
ENDIF (${COMMONS_FILE} STREQUAL NOTFOUND)

# Conan Packaging support
if(APPLE)
    include(mac.conan/conanbuildinfo.cmake)
else(APPLE)
    include(.conan/conanbuildinfo.cmake)
endif(APPLE)
conan_basic_setup()

set(RELEASE_STRING "${RELEASE_MAJOR}.${RELEASE_MINOR}.${RELEASE_PATCH}-${BUILD_ID}")
message(STATUS "Building Release: ${RELEASE_STRING}")


set(SOURCE_DIR ${PROJECT_SOURCE_DIR}/src)
set(INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include)
set(TESTS_DIR ${PROJECT_SOURCE_DIR}/tests)
set(PROTO_DIR ${PROJECT_SOURCE_DIR}/proto)
set(PROJECT_BINARY_DIR ${PROJECT_SOURCE_DIR}/build)

# Configure a header file to pass settings into source code.
configure_file(
    ${SOURCE_DIR}/config.h.in
    ${PROJECT_BINARY_DIR}/version.h
)


include_directories(
        ${INCLUDE_DIR}
        ${PROJECT_BINARY_DIR}
        ${INSTALL_DIR}/include
)

link_directories(${INSTALL_DIR}/lib)

set(SOURCES
        ${SOURCE_DIR}/api/rest/ApiServer.cpp
)

set(LIBS
        glog
        microhttpd
        pthread
)

##
# libapiserver.so
#
# Shared (dynamic) library for the SWIM Gossip Protocol.
#
add_library(${PROJECT_NAME} SHARED
        ${SOURCES}
)
target_link_libraries(${PROJECT_NAME} ${LIBS})


if (NOT ${INSTALL_DIR})
    message(STATUS "Installing lib${PROJECT_NAME}.so shared library to ${INSTALL_DIR}/lib "
    "and include files to ${INSTALL_DIR}/include/${PROJECT_NAME}")
    install(TARGETS apiserver DESTINATION ${INSTALL_DIR}/lib)

    # Generated configuration file, containing version and build number.
    install(FILES ${PROJECT_BINARY_DIR}/version.h DESTINATION
            "${INSTALL_DIR}/include/${PROJECT_NAME}")

    # Install Library headers.
    install(DIRECTORY ${INCLUDE_DIR}/ DESTINATION ${INSTALL_DIR}/include/${PROJECT_NAME}
            FILES_MATCHING PATTERN "*.h" PATTERN "*.hpp")

else (NOT ${INSTALL_DIR})
    message(WARNING "INSTALL_DIR is not defined, files will not be installed. "
                    "Use -DINSTALL_DIR=/path/to/install to enable")
endif (NOT ${INSTALL_DIR})

# Unit tests build is defined in the tests/ folder
add_subdirectory(${TESTS_DIR})

#########
# Examples - demo execs/programs to show usage of the libraries/utilities.
#########

set(EXAMPLES_DIR ${SOURCE_DIR}/examples)

# Examples will use the latest build for the shared libraries
link_directories(
        ${CMAKE_BINARY_DIR}
        ${INSTALL_DIR}/lib
)

# Demo server to either listen to incoming pings, or send.
#
include_directories(${INSTALL_DIR}/include)
add_executable(server_demo
        ${SOURCES}
        ${EXAMPLES_DIR}/server_demo.cpp
)
target_link_libraries(server_demo
        ${LIBS}
)
