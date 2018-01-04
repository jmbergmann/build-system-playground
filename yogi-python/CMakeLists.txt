#===== Tests =====
add_test(
  NAME yogi-python
  COMMAND python3 -m unittest
  WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
)
