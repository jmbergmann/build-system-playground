# Tests
add_test (
  NAME yogi-python-test
  COMMAND ${PYTHON_EXECUTABLE} -m unittest
  WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
)

set_property (TEST yogi-python-test PROPERTY
  ENVIRONMENT "YOGI_CORE_LIBRARY=$<TARGET_FILE:yogi-core>"
)
