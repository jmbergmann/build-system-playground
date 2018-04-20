cmake_minimum_required (VERSION 3.2 FATAL_ERROR)

# Read version number from yogi_core.h
file (STRINGS "${CMAKE_SOURCE_DIR}/yogi-core/include/yogi_core.h" core_version
  REGEX "#define YOGI_HDR_VERSION +\".*$")
string (REGEX MATCH "[0-9]+\\.[0-9]+\\.[0-9]+" core_version ${core_version})

message (STATUS "Yogi version: ${core_version}")
project (yogi VERSION ${core_version} LANGUAGES CXX)

# Include CMake modules
set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")

include (cotire)
include (GNUInstallDirs)

# Boost
set (Boost_USE_STATIC_LIBS ON)
set (Boost_MULTITHREADED ON)
find_package (Boost 1.65 COMPONENTS system filesystem thread REQUIRED)
add_library (boost INTERFACE IMPORTED)
set_property (TARGET boost
  PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${Boost_INCLUDE_DIR})
find_package (Threads REQUIRED)

# Read required Python version from setup.py
file (STRINGS "${CMAKE_SOURCE_DIR}/yogi-python/setup.py" python_version
  REGEX "'python_requires'.*$")
string (REGEX MATCH "[0-9]+\\.[0-9]+" python_version ${python_version})
find_package (PythonInterp ${python_version} REQUIRED)

# Testing
enable_testing ()
find_package (GTest REQUIRED)
include_directories (${GTEST_INCLUDE_DIRS})
add_definitions (-D_SILENCE_TR1_NAMESPACE_DEPRECATION_WARNING)

# Include sub-projects
add_subdirectory (yogi-core)
add_subdirectory (yogi-cpp)
add_subdirectory (yogi-hub)
add_subdirectory (yogi-python)
add_subdirectory (yogi-dotnet)
add_subdirectory (yogi-javascript)
