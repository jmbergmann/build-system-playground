cmake_minimum_required(VERSION 3.2 FATAL_ERROR)

# Read version number from yogi_core.h
file(STRINGS "${CMAKE_SOURCE_DIR}/yogi-core/include/yogi_core.h" core_version
  REGEX "#define YOGI_VERSION +\".*$")
string(REGEX MATCH "[0-9]+\\.[0-9]+\\.[0-9]+" core_version ${core_version})

message("Yogi version: ${core_version}")
project(yogi VERSION ${core_version} LANGUAGES CXX)

# Include CMake modules
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")

include(cotire)
include(GNUInstallDirs)

# Boost
find_package(Boost REQUIRED)
add_library(boost INTERFACE IMPORTED)
set_property(TARGET boost PROPERTY
  INTERFACE_INCLUDE_DIRECTORIES ${Boost_INCLUDE_DIR})
find_package(Threads REQUIRED)

# Python
find_package(PythonInterp 3.6 REQUIRED)

# Testing
enable_testing()
find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

# Include sub-projects
add_subdirectory(yogi-core)
add_subdirectory(yogi-cpp)
add_subdirectory(yogi-hub)
add_subdirectory(yogi-python)
add_subdirectory(yogi-javascript)
