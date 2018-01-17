# Tests
add_test (
  NAME yogi-python
  COMMAND ${PYTHON_EXECUTABLE} -m unittest
  WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
)

set_property (TEST yogi-python PROPERTY
  ENVIRONMENT "YOGI_CORE_LIBRARY=$<TARGET_FILE:yogi-core>"
)
