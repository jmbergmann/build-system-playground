# Tests
add_test (
  NAME yogi-dotnet
  COMMAND dotnet test
  WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/test
)

set_property (TEST yogi-dotnet PROPERTY
  ENVIRONMENT "YOGI_CORE_LIBRARY=$<TARGET_FILE:yogi-core>"
)
