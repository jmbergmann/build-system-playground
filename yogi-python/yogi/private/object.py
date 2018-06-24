from .library import yogi
from .errors import Failure, api_result_handler

from ctypes import c_void_p


yogi.YOGI_Destroy.restype = api_result_handler
yogi.YOGI_Destroy.argtypes = [c_void_p]


class Object:
    """Base class for all "creatable" objects.
    """
    def __init__(self, handle: c_void_p):
        self._handle = handle

    def __del__(self):
        try:
            yogi.YOGI_Destroy(self._handle)
        except Failure:
            raise AssertionError("Should never get here")

    def __str__(self):
        s = self.__class__.__name__
        if self._handle.value:
            s += " {:#010x}".format(self._handle.value)
        else:
            s += " [INVALID]"
        return s
