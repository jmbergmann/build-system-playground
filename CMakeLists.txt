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

# Windows-specific definitions
if (WIN32)
  add_definitions(-D_WIN32_WINNT=0x0601) # Windows 7
  add_definitions(-D_SCL_SECURE_NO_WARNINGS)
  add_definitions(-D_CRT_SECURE_NO_WARNINGS)
  add_definitions(-D_WINSOCK_DEPRECATED_NO_WARNINGS)
endif ()

# OpenSSL
find_package (OpenSSL REQUIRED)

# Boost
set (Boost_USE_STATIC_LIBS ON)
set (Boost_MULTITHREADED ON)
find_package (Boost 1.66
  COMPONENTS system filesystem thread date_time program_options
  REQUIRED)
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

# Clang-Tidy
find_program (CLANG_TIDY_EXE NAMES "clang-tidy"
  DOC "Path to clang-tidy executable")

if (NOT CLANG_TIDY_EXE)
  message (STATUS "clang-tidy not found.")
else ()
  message (STATUS "clang-tidy found: ${CLANG_TIDY_EXE}")
  set (CLANG_TIDY_CMD "${CLANG_TIDY_EXE}" "-checks=*,-clang-analyzer-alpha.* -- -I${Boost_INCLUDE_DIR}")
endif ()

function (run_clang_tidy target)
  if (CLANG_TIDY_EXE)
    message (STATUS "clang-tidy disabled for now because it cannot find boost headers on Windows")
    # set_target_properties (${target} PROPERTIES CXX_CLANG_TIDY "${CLANG_TIDY_CMD}")
  endif ()
endfunction ()

# Include sub-projects
add_subdirectory (yogi-core)
add_subdirectory (yogi-cpp)
add_subdirectory (yogi-hub)
add_subdirectory (yogi-python)
add_subdirectory (yogi-dotnet)
add_subdirectory (yogi-javascript)
