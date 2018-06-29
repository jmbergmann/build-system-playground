# Clang-Tidy

find_program (CLANG_TIDY_EXE NAMES "clang-tidy"
  DOC "Path to clang-tidy executable")

if (NOT CLANG_TIDY_EXE)
  message (STATUS "clang-tidy not found.")
else ()
  message (STATUS "clang-tidy found: ${CLANG_TIDY_EXE}")
  set (CLANG_TIDY_CMD "${CLANG_TIDY_EXE}" "-checks=*,-clang-analyzer-alpha.* -- -I${Boost_INCLUDE_DIR}")
endif ()

macro (run_clang_tidy target)
  if (CLANG_TIDY_EXE)
    set_target_properties (${target} PROPERTIES CXX_CLANG_TIDY "${CLANG_TIDY_CMD}")
  endif ()
endmacro ()
