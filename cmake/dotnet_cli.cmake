# Dotnet CLI

find_program (DOTNET_EXECUTABLE "dotnet")

execute_process (
  COMMAND ${DOTNET_EXECUTABLE} --info
  COMMAND grep -e "RID:.*-.*"
  COMMAND sed "s/.*-//"
  OUTPUT_VARIABLE dotnet_architecture
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

message (STATUS "Dotnet CLI architecture is ${dotnet_architecture}")
