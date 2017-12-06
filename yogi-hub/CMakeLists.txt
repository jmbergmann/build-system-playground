add_library(yogi-hub-static STATIC
    src/random_stuff.cc)

cotire(yogi-hub-static)

add_executable(yogi-hub
    src/main.cc)

target_link_libraries(yogi-hub
    yogi-cpp)

cotire(yogi-hub)

add_executable(yogi-hub-test
    test/yogi_hub_test.cc)

target_link_libraries(yogi-hub-test
    yogi-cpp
    ${GTEST_BOTH_LIBRARIES}
    Threads::Threads)

cotire(yogi-hub-test)

add_test(yogi-hub-test yogi-hub-test)
