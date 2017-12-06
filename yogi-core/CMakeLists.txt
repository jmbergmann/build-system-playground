add_library(yogi-core-static STATIC
    src/errors.cc)

cotire(yogi-core-static)

add_library(yogi-core SHARED
    src/yogi_core.cc)

target_link_libraries(yogi-core
    yogi-core-static)

cotire(yogi-core)

target_include_directories(yogi-core PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
    PRIVATE src)

export(TARGETS yogi-core-static FILE YogiCoreStaticConfig.cmake)
export(TARGETS yogi-core FILE YogiCoreConfig.cmake)

add_executable(yogi-core-test
    test/errors_test.cc
    test/version_test.cc)

target_link_libraries(yogi-core-test
    yogi-core-static
    yogi-core
    ${GTEST_BOTH_LIBRARIES})

cotire(yogi-core-test)

add_test(yogi-core-test yogi-core-test)
