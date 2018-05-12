# Static library
add_library (yogi-hub-static STATIC
  src/random_stuff.cc
)

cotire (yogi-hub-static)

# Executable
add_executable (yogi-hub
  src/main.cc
)

target_link_libraries (yogi-hub
  yogi-cpp
)

cotire (yogi-hub)
run_clang_tidy (yogi-hub)

# Tests
add_executable (yogi-hub-test
  test/yogi_hub_test.cc
)

target_link_libraries (yogi-hub-test
  yogi-cpp
  ${GTEST_BOTH_LIBRARIES}
  Threads::Threads
)

cotire (yogi-hub-test)
run_clang_tidy (yogi-hub-test)

add_test (
  NAME yogi-hub-test
  COMMAND yogi-hub-test
)

set_property (TEST yogi-hub-test PROPERTY
  ENVIRONMENT "YOGI_CORE_LIBRARY=$<TARGET_FILE:yogi-core>"
)
