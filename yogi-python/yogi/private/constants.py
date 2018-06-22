from .errors import *
from ctypes import *
from .logging import Verbosity
from . import library  # To make sure the library is loaded first


yogi.YOGI_GetConstant.restype = api_result_handler
yogi.YOGI_GetConstant.argtypes = [c_void_p, c_int]


def get_constant(api_id, api_type):
    c = api_type()
    yogi.YOGI_GetConstant(byref(c), api_id)
    return c.value.decode() if api_type is c_char_p else c.value


class Constants:
    VERSION_NUMBER = get_constant(1, c_char_p)  # type: str
    VERSION_MAJOR = get_constant(2, c_int)  # type: int
    VERSION_MINOR = get_constant(3, c_int)  # type: int
    VERSION_PATCH = get_constant(4, c_int)  # type: int
    DEFAULT_ADV_ADDRESS = get_constant(5, c_char_p)  # type: str
    DEFAULT_ADV_PORT = get_constant(6, c_int)  # type: int
    DEFAULT_ADV_INTERVAL = get_constant(7, c_longlong)  # type: int
    DEFAULT_CONNECTION_TIMEOUT = get_constant(8, c_longlong)  # type: int
    DEFAULT_LOGGER_VERBOSITY = Verbosity(
        get_constant(9, c_int))  # type: Verbosity
    DEFAULT_LOG_TIME_FORMAT = get_constant(10, c_char_p)  # type: str
    DEFAULT_LOG_FORMAT = get_constant(11, c_char_p)  # type: str
    MAX_MESSAGE_SIZE = get_constant(12, c_int)  # type: int
