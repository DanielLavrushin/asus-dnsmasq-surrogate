cmake_minimum_required (VERSION 3.5)

project(dnsmasq-surrogate
  VERSION 2.0.0
  LANGUAGES C CXX
)

set(CMAKE_MODULE_PATH
    ${CMAKE_MODULE_PATH}
    ${PROJECT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/CMakeModules
)

set(BUILD_DIST true CACHE BOOL
    "Whether to build package for Asus target. When false, will build tests for host.")
set(CXX_FLAGS "--std=c++17" CACHE STRING
    "Compile flags for Distribution and Tests.")

# Add cross-compilation settings
if (BUILD_DIST)
    set(CMAKE_SYSTEM_NAME Linux)
    set(CMAKE_SYSTEM_PROCESSOR arm)
    set(CMAKE_C_COMPILER arm-linux-gnueabi-gcc)
    set(CMAKE_CXX_COMPILER arm-linux-gnueabi-g++)

    # Set linker flags to use an older glibc version or to link statically
    set(CMAKE_CXX_FLAGS "--std=c++17 -static-libstdc++ -static-libgcc -static")
    set(CMAKE_SHARED_LINKER_FLAGS "-static -Wl,--whole-archive -lc -Wl,--no-whole-archive")
    set(CMAKE_EXE_LINKER_FLAGS "-static -Wl,-Bstatic")

    set(CMAKE_FIND_ROOT_PATH /usr/arm-linux-gnueabi)
    set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
    set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
    set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
    set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

    include(SurrogateDist)
else()
    include(SurrogateTest)
endif()
