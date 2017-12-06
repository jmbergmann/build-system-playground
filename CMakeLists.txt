cmake_minimum_required(VERSION 3.2 FATAL_ERROR)
project(yogi VERSION 0.0.3 LANGUAGES CXX)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")

include(cotire)
include(GNUInstallDirs)

find_package(Boost REQUIRED)
add_library(boost INTERFACE IMPORTED)
set_property(TARGET boost PROPERTY
    INTERFACE_INCLUDE_DIRECTORIES ${Boost_INCLUDE_DIR})

enable_testing()
find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

add_subdirectory(yogi-core)
add_subdirectory(yogi-cpp)
add_subdirectory(yogi-hub)
