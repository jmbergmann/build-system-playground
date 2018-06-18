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
add_executable (yogi-cpp-test
  test/library_test.cc
)

target_link_libraries (yogi-cpp-test
  yogi-cpp
  ${GTEST_BOTH_LIBRARIES}
  Threads::Threads
)

cotire (yogi-cpp-test)
run_clang_tidy (yogi-cpp-test)

add_test (
  NAME yogi-cpp-test
  COMMAND yogi-cpp-test
)

set_property (TEST yogi-cpp-test PROPERTY
  ENVIRONMENT "YOGI_CORE_LIBRARY=$<TARGET_FILE:yogi-core>"
)
