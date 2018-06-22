import os
import platform
import ctypes


# Determine shared library path and filename
lib_filename = os.getenv('YOGI_CORE_LIBRARY')
if lib_filename is None:
    if platform.system() == 'Windows':
        lib_filename = "yogi-core.dll"
    elif platform.system() == 'Linux':
        lib_filename = "libyogi-core.so"
    else:
        raise Exception(platform.system() + ' is not supported')

# Load the shared library
try:
    yogi = ctypes.cdll.LoadLibrary(lib_filename)
except Exception as e:
    raise Exception('ERROR: Could not load {}: {}. Make sure the library is in your library search path.'
                    .format(lib_filename, e))
