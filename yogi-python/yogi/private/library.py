import platform
import os
import posixpath
from ctypes import *


# ======================================================================================================================
# Load the shared library
# ======================================================================================================================
lib_filename = None
if platform.system() == 'Windows':
    lib_filename = "yogi-core.dll"
elif platform.system() == 'Linux':
    lib_filename = "libyogi_core.so"
else:
    raise Exception(platform.system() + ' is not supported')

lib_path = os.getenv('YOGI_CORE_LIBRARY_PATH', lib_filename).replace('\\', '/')
if not lib_path.endswith(('.dll', '.so')):
    lib_path= posixpath.join(lib_path, lib_filename)
        
try:
    print(lib_path)
    print(os.getenv('YOGI_CORE_LIBRARY_PATH'))
    print(os.getcwd())
    yogi = cdll.LoadLibrary(lib_path)
except Exception as e:
    raise Exception('ERROR: Could not load {}: {}. Make sure the library is in your library search path.'
                    .format(lib_path, e))
