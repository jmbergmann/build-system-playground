cmake_minimum_required (VERSION 2.8)
project (yogi-core-tests)

file (GLOB_RECURSE files *.h *.cc)
add_executable (yogi-core-tests ${files})
