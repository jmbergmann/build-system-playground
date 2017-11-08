add_library(yogi-cpp INTERFACE)

target_link_libraries(yogi-cpp INTERFACE yogi-core)

target_include_directories(yogi-cpp INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>)

export(TARGETS yogi-cpp FILE YogiCppConfig.cmake)

add_executable(yogi-cpp-test
    test/yogi_cpp_test.cc)

target_link_libraries(yogi-cpp-test
    yogi-cpp)

add_test(yogi-cpp-test yogi-cpp-test)
