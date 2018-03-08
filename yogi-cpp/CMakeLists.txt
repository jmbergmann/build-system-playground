# Static library
add_library (yogi-cpp INTERFACE)

target_link_libraries (yogi-cpp INTERFACE
  yogi-core-header
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

add_test (
  NAME yogi-cpp-test
  COMMAND yogi-cpp-test
)

set_property (TEST yogi-cpp-test PROPERTY
  ENVIRONMENT "YOGI_CORE_LIBRARY=$<TARGET_FILE:yogi-core>"
)
