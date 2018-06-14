# Compiler flags
if (WIN32)
  set (CMAKE_CXX_FLAGS             "/EHsc /permissive- /W4 /w14640")
else ()
  set (warnings                  "-Wall -Wextra -Wnon-virtual-dtor -pedantic -Wold-style-cast -Wcast-align -Wunused -Woverloaded-virtual -Wconversion -Wsign-conversion -Wduplicated-cond -Wlogical-op -Wnull-dereference")
  set (ignored_warnings          "-Wno-unused-variable -Wno-unused-function -Wno-overloaded-virtual -Wno-unused-value -Wno-shadow -Wno-trigraphs")
  set (export_map                "-Wl,--version-script='${CMAKE_CURRENT_LIST_DIR}/src/export.map'")
  set (CMAKE_CXX_STANDARD        14)
  set (CMAKE_CXX_FLAGS           "-std=c++${CMAKE_CXX_STANDARD} ${warnings} ${ignored_warnings} -fvisibility=hidden -fPIC -DBOOST_ALL_DYN_LINK")
  set (CMAKE_CXX_FLAGS_DEBUG     "-g -ggdb")
  set (CMAKE_CXX_FLAGS_RELEASE   "-O2 -DNDEBUG")
  set (CMAKE_SHARED_LINKER_FLAGS "${export_map}")
endif ()

# Static library
add_library (yogi-core-static STATIC
  src/api/constants.cc
  src/api/error.cc
  src/api/object.cc
  src/licenses/3rd_party_licenses.cc
  src/licenses/yogi_license.cc
  src/objects/detail/branch/advertising_receiver.cc
  src/objects/detail/branch/advertising_sender.cc
  src/objects/detail/branch/branch_connection.cc
  src/objects/detail/branch/branch_info.cc
  src/objects/detail/branch/connection_manager.cc
  src/objects/detail/configuration/command_line_parser.cc
  src/objects/detail/configuration/glob.cc
  src/objects/detail/log/console_log_sink.cc
  src/objects/detail/log/file_log_sink.cc
  src/objects/detail/log/hook_log_sink.cc
  src/objects/detail/log/log_sink.cc
  src/objects/detail/log/text_based_log_sink.cc
  src/objects/branch.cc
  src/objects/configuration.cc
  src/objects/context.cc
  src/objects/logger.cc
  src/objects/timer.cc
  src/utils/console.cc
  src/utils/crypto.cc
  src/utils/ip.cc
  src/utils/socket.cc
  src/utils/system.cc
  src/utils/timestamp.cc
)

cotire (yogi-core-static)
run_clang_tidy (yogi-core-static)

export (TARGETS yogi-core-static
  FILE YogiCoreStaticConfig.cmake
)

# Dynamic library
add_library (yogi-core SHARED
  src/yogi_core/branches.cc
  src/yogi_core/configurations.cc
  src/yogi_core/contexts.cc
  src/yogi_core/helpers.cc
  src/yogi_core/logging.cc
  src/yogi_core/miscellaneous.cc
  src/yogi_core/signals.cc
  src/yogi_core/timers.cc
)

target_link_libraries (yogi-core
  yogi-core-static
  ${Boost_LIBRARIES}
  OpenSSL::Crypto
)

if (WIN32)
  target_link_libraries (yogi-core
    bcrypt.lib
  )
endif ()

cotire (yogi-core)
run_clang_tidy (yogi-core)

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
  test/common.cc
  test/configuration_test.cc
  test/connection_manager_test.cc
  test/constants_test.cc
  test/context_test.cc
  test/errors_test.cc
  test/licenses_test.cc
  test/logger_test.cc
  test/object_test.cc
  test/serialize_test.cc
  test/signals_test.cc
  test/time_test.cc
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
run_clang_tidy (yogi-core-test)

add_test (yogi-core-test yogi-core-test)

# Valgrind
add_custom_target(yogi-core-test-valgrind
  COMMAND valgrind --leak-check=yes $<TARGET_FILE:yogi-core-test>
  DEPENDS yogi-core-test
)

add_custom_target(yogi-core-test-helgrind
  COMMAND valgrind --tool=helgrind $<TARGET_FILE:yogi-core-test>
  DEPENDS yogi-core-test
)
