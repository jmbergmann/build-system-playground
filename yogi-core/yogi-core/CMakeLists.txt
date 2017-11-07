cmake_minimum_required (VERSION 2.8)
project (yogi-core)

file (GLOB_RECURSE files *.h *.cc)
add_library (yogi-core SHARED ${files})
