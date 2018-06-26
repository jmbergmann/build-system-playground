from .library import yogi
from .errors import Failure, ErrorCode, api_result_handler

from ctypes import c_void_p
from typing import List, Any


yogi.YOGI_Destroy.restype = api_result_handler
yogi.YOGI_Destroy.argtypes = [c_void_p]


class Object:
    """Base class for all "creatable" objects.
    """

    def __init__(self, handle: c_void_p, dependecies: List[Any] = []):
        """Constructs the object.

        The objects in the depencies list are stored in order to prevent the
        garbage collector to destroy them before this objects gets destroyed.
        For example, it would prevent the destruction of the context object in
        the following case:
            timer = yogi.Timer(yogi.Context())

        Args:
            handle:       Native handle representing the object.
            dependencies: Other objects that this object depends on.
        """
        self._handle = handle
        self._dependencies = dependecies

    def __del__(self):
        assert self._handle, "Looks like the ctor of the inheriting class" \
            "forgot to call Object.__init__()."

        try:
            yogi.YOGI_Destroy(self._handle)
        except Failure as failure:
            info = ""
            if failure.error_code is ErrorCode.OBJECT_STILL_USED:
                info = " Check that you don't have circular dependencies on" \
                    " Yogi objects."

            raise AssertionError("Could not destroy {} instance: {}.{}"
                                 .format(self.__class__.__name__, failure,
                                         info))

    def __str__(self):
        s = self.__class__.__name__
        if self._handle.value:
            s += " {:#010x}".format(self._handle.value)
        else:
            s += " [INVALID]"
        return s
