project(yogi-hub VERSION 0.0.3 LANGUAGES CXX)

add_library(yogi-hub-static STATIC
    src/random_stuff.cc)

add_executable(yogi-hub
    src/main.cc)

target_link_libraries(yogi-hub
    yogi-cpp)

add_executable(yogi-hub-test
    test/yogi_hub_test.cc)

target_link_libraries(yogi-hub-test
    yogi-cpp)

add_test(yogi-hub-test yogi-hub-test)
