# Static library
add_library (yogi-core-static STATIC
  src/constants.cc
  src/errors.cc
)

cotire (yogi-core-static)

export (TARGETS yogi-core-static
  FILE YogiCoreStaticConfig.cmake
)

# Dynamic library
add_library (yogi-core SHARED
  src/yogi_core.cc
)

target_link_libraries (yogi-core
  yogi-core-static
)

cotire (yogi-core)

target_include_directories (yogi-core PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
  PRIVATE src
)

export (TARGETS yogi-core
  FILE YogiCoreConfig.cmake
)

# Core header only
add_library (yogi-core-header INTERFACE)

target_include_directories (yogi-core-header INTERFACE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

export (TARGETS yogi-core-header
  FILE YogiCoreHeaderConfig.cmake
)

# Tests
add_executable (yogi-core-test
  test/errors_test.cc
  test/version_test.cc
  test/constants_test.cc
)

target_link_libraries (yogi-core-test
  yogi-core-static
  yogi-core
  ${GTEST_BOTH_LIBRARIES}
  Threads::Threads
)

cotire (yogi-core-test)

add_test (yogi-core-test yogi-core-test)
