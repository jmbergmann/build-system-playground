# Compiler flags
if (NOT WIN32)
  set (ignored_warnings          "-Wno-unused-variable -Wno-unused-function -Wno-overloaded-virtual -Wno-unused-value")
  set (export_map                "-Wl,--version-script='${CMAKE_CURRENT_LIST_DIR}/src/export.map'")
  set (CMAKE_CXX_STANDARD        14)
  set (CMAKE_CXX_FLAGS           "-std=c++${CMAKE_CXX_STANDARD} -Wall -Werror ${ignored_warnings} -fvisibility=hidden -fPIC -DBOOST_ALL_DYN_LINK")
  set (CMAKE_CXX_FLAGS_DEBUG     "-g -ggdb")
  set (CMAKE_CXX_FLAGS_RELEASE   "-O2 -DNDEBUG")
  set (CMAKE_SHARED_LINKER_FLAGS "${export_map}")
endif ()

# Static library
add_library (yogi-core-static STATIC
  src/api/constants.cc
  src/api/error.cc
  src/api/object.cc
  src/objects/branch.cc
  src/objects/context.cc
  src/objects/timer.cc
  src/utils/system.cc
  src/utils/time.cc
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
  OpenSSL::Crypto
  ${Boost_LIBRARIES}
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
  test/branch_test.cc
  test/constants_test.cc
  test/context_test.cc
  test/errors_test.cc
  test/object_test.cc
  test/timer_test.cc
  test/version_test.cc
)

target_link_libraries (yogi-core-test
  yogi-core-static
  yogi-core
  ${GTEST_BOTH_LIBRARIES}
  Threads::Threads
)

cotire (yogi-core-test)

add_test (yogi-core-test yogi-core-test)
