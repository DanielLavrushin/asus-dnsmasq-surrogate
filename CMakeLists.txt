#!/usr/bin/ccmake .
cmake_minimum_required (VERSION 3.5)

project(dnsmasq-surrogate
  VERSION 2.2.0
  LANGUAGES C CXX
)

set(CMAKE_MODULE_PATH
    ${CMAKE_MODULE_PATH}
    ${PROJECT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/CMakeModules
)

set(BUILD_DIST false CACHE BOOL
    "Whether to build package for Asus target. When false, will build tests for host.")
set(CXX_FLAGS "--std=c++17" CACHE STRING
    "Compile flags for Distribution and Tests.")

if (BUILD_DIST)
    set(CMAKE_INSTALL_RPATH "/lib")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--dynamic-linker=/lib/ld-linux.so.3")
    set(CMAKE_CXX_FLAGS    "-Wl,-rpath,/lib -s ${CXX_FLAGS}")
    include(SurrogateDist)
else()
    include(SurrogateTest)
endif()