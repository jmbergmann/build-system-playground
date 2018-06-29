# Compiler flags
if (WIN32)
  set (CMAKE_CXX_FLAGS "/EHsc /permissive- /W4 /w14640")
  use_static_windows_runtime ()

  add_definitions(
    -D_WIN32_WINNT=0x0601 # Windows 7
    -D_SCL_SECURE_NO_WARNINGS
    -D_CRT_SECURE_NO_WARNINGS
    -D_WINSOCK_DEPRECATED_NO_WARNINGS
  )
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

# OpenSSL
set (OPENSSL_USE_STATIC_LIBS ON)
find_package (OpenSSL REQUIRED)

# Boost
set (Boost_USE_STATIC_LIBS ON)
set (Boost_USE_STATIC_RUNTIME OFF)
set (Boost_USE_DEBUG_RUNTIME ON)
set (Boost_MULTITHREADED ON)
find_package (Boost 1.66
  COMPONENTS system filesystem thread date_time program_options
  REQUIRED)
add_library (boost INTERFACE IMPORTED)
set_property (TARGET boost
  PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${Boost_INCLUDE_DIR})
find_package (Threads REQUIRED)

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
  src/objects/detail/command_line_parser.cc
  src/objects/detail/log/console_log_sink.cc
  src/objects/detail/log/file_log_sink.cc
  src/objects/detail/log/hook_log_sink.cc
  src/objects/detail/log/log_sink.cc
  src/objects/detail/log/text_based_log_sink.cc
  src/objects/branch.cc
  src/objects/configuration.cc
  src/objects/context.cc
  src/objects/logger.cc
  src/objects/signal_set.cc
  src/objects/timer.cc
  src/utils/console.cc
  src/utils/crypto.cc
  src/utils/glob.cc
  src/utils/ip.cc
  src/utils/socket.cc
  src/utils/system.cc
  src/utils/timestamp.cc
)

target_include_directories (yogi-core-static
  PUBLIC ${Boost_INCLUDE_DIR}
)

cotire (yogi-core-static)

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

if (WIN32)
  set_target_properties (yogi-core PROPERTIES
    OUTPUT_NAME "yogi-core-${TARGET_ARCHITECTURE}"
  )
endif()

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
find_package (GTest REQUIRED)
include_directories (${GTEST_INCLUDE_DIRS})
add_definitions (-D_SILENCE_TR1_NAMESPACE_DEPRECATION_WARNING)

add_executable (yogi-core-test
  test/api/constants_test.cc
  test/api/errors_test.cc
  test/api/object_test.cc
  test/licenses/licenses_test.cc
  test/objects/branch_test.cc
  test/objects/command_line_parser_test.cc
  test/objects/configuration_test.cc
  test/objects/connection_manager_test.cc
  test/objects/context_test.cc
  test/objects/logger_test.cc
  test/objects/signal_set_test.cc
  test/objects/timer_test.cc
  test/utils/glob_test.cc
  test/utils/serialize_test.cc
  test/common.cc
  test/time_test.cc
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

# Valgrind
add_custom_target(yogi-core-test-valgrind
  COMMAND valgrind --leak-check=yes $<TARGET_FILE:yogi-core-test>
  DEPENDS yogi-core-test
)

add_custom_target(yogi-core-test-helgrind
  COMMAND valgrind --tool=helgrind $<TARGET_FILE:yogi-core-test>
  DEPENDS yogi-core-test
)
