project(yogi-core VERSION 0.0.3 LANGUAGES CXX)

add_library(lib STATIC
    src/stdafx.cc
    src/errors.cc)

add_library(yogi-core SHARED
    src/yogi_core.cc)

target_include_directories(lib PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
    PRIVATE src)

export(TARGETS yogi-core FILE YogiCoreConfig.cmake)

add_executable(yogi-core-test
    test/yogi_core_test.cc)

target_link_libraries(yogi-core-test
    lib
    yogi-core)

add_test(yogi-core-test yogi-core-test)
