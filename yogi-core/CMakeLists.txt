project(yogi-core VERSION 0.0.3 LANGUAGES CXX)

add_library(yogi-core-static STATIC
    src/stdafx.cc
    src/errors.cc)

add_library(yogi-core SHARED
    src/yogi_core.cc)

target_link_libraries(yogi-core
    yogi-core-static)

target_include_directories(yogi-core PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
    PRIVATE src)

export(TARGETS yogi-core-static FILE YogiCoreStaticConfig.cmake)
export(TARGETS yogi-core FILE YogiCoreConfig.cmake)

add_executable(yogi-core-test
    test/yogi_core_test.cc)

target_link_libraries(yogi-core-test
    yogi-core-static
    yogi-core)

add_test(yogi-core-test yogi-core-test)
