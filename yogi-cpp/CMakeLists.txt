# Compiler flags
if (WIN32)
  set (CMAKE_CXX_FLAGS "/EHsc /permissive- /W4 /w14640")
  use_static_windows_runtime ()
else ()
  set (warnings                  "-Wall -Wextra -Wnon-virtual-dtor -pedantic -Wold-style-cast -Wcast-align -Wunused -Woverloaded-virtual -Wconversion -Wsign-conversion -Wduplicated-cond -Wlogical-op -Wnull-dereference")
  set (ignored_warnings          "-Wno-unused-variable -Wno-unused-function -Wno-overloaded-virtual -Wno-unused-value -Wno-shadow -Wno-trigraphs")
  set (CMAKE_CXX_STANDARD        14)
  set (CMAKE_CXX_FLAGS           "-std=c++${CMAKE_CXX_STANDARD} ${warnings} ${ignored_warnings}")
  set (CMAKE_CXX_FLAGS_DEBUG     "-g -ggdb")
  set (CMAKE_CXX_FLAGS_RELEASE   "-O2 -DNDEBUG")
endif ()

# Interface library
add_library (yogi-cpp INTERFACE)

target_link_libraries (yogi-cpp INTERFACE
  yogi-core-header
  ${CMAKE_DL_LIBS}
)

target_include_directories (yogi-cpp INTERFACE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)

export (TARGETS yogi-cpp
  FILE YogiCppConfig.cmake
)

# Tests
find_package (GTest REQUIRED)
include_directories (${GTEST_INCLUDE_DIRS})
add_definitions (-D_SILENCE_TR1_NAMESPACE_DEPRECATION_WARNING)

add_executable (yogi-cpp-test
  test/common.cc
  test/constants_compile_test.cc
  test/constants_test.cc
  test/context_compile_test.cc
  test/context_test.cc
  test/errors_compile_test.cc
  test/errors_test.cc
  test/io_compile_test.cc
  test/io_test.cc
  test/licenses_compile_test.cc
  test/licenses_test.cc
  test/logging_compile_test.cc
  test/logging_test.cc
  test/object_compile_test.cc
  test/object_test.cc
  test/stream_io_compile_test.cc
  test/stream_io_test.cc
  test/time_compile_test.cc
  test/time_test.cc
  test/version_compile_test.cc
  test/version_test.cc
  test/yogi_header_test.cc
)

target_link_libraries (yogi-cpp-test
  yogi-cpp
  ${GTEST_BOTH_LIBRARIES}
  Threads::Threads
)

cotire (yogi-cpp-test)

add_test (
  NAME yogi-cpp-test
  COMMAND yogi-cpp-test
)

set_property (TEST yogi-cpp-test PROPERTY
  ENVIRONMENT "YOGI_CORE_LIBRARY=$<TARGET_FILE:yogi-core>"
)
