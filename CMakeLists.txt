cmake_minimum_required (VERSION 3.2 FATAL_ERROR)

set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")

include (check_toolchain)
include (yogi_version)

project (yogi VERSION ${YOGI_VERSION} LANGUAGES CXX)

include (cotire)
include (GNUInstallDirs)
include (static_windows_runtime)
include (python_interpreter)
include (dotnet_cli)

enable_testing ()

add_subdirectory (yogi-core)
add_subdirectory (yogi-cpp)
add_subdirectory (yogi-hub)
add_subdirectory (yogi-python)
add_subdirectory (yogi-dotnet)
add_subdirectory (yogi-javascript)
