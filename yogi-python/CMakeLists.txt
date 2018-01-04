#===== Tests =====
add_test(
  NAME yogi-python
  COMMAND ${PYTHON_EXECUTABLE} -m unittest
  WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
)
