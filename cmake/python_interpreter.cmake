# Finds Python interpreter based on required Python version from setup.py

file (STRINGS "${CMAKE_SOURCE_DIR}/yogi-python/setup.py" python_version
  REGEX "'python_requires'.*$")
string (REGEX MATCH "[0-9]+\\.[0-9]+" python_version ${python_version})
find_package (PythonInterp ${python_version} REQUIRED)

execute_process (
  COMMAND ${PYTHON_EXECUTABLE} -c "import sys; print('x64' if sys.maxsize > 2**32 else 'x86')"
  OUTPUT_VARIABLE python_architecture
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

message (STATUS "Python interpreter architecture is ${python_architecture}")
