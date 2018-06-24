from .errors import api_result_handler
from .logging import Verbosity
from .library import yogi

from ctypes import c_void_p, c_int, c_char_p, c_longlong, byref


yogi.YOGI_GetConstant.restype = api_result_handler
yogi.YOGI_GetConstant.argtypes = [c_void_p, c_int]


def get_constant(api_id, api_type):
    c = api_type()
    yogi.YOGI_GetConstant(byref(c), api_id)
    return c.value.decode() if api_type is c_char_p else c.value


class Constants:
    """Constants built into the Yogi Core library.

    Attributes:
        VERSION_NUMBER             Complete Yogi Core version number.
        VERSION_MAJOR              Yogi Core major version number.
        VERSION_MINOR              Yogi Core minor version number.
        VERSION_PATCH              Yogi Core patch version number.
        DEFAULT_ADV_ADDRESS        Default IP address for advertising.
        DEFAULT_ADV_PORT           Default UDP port for advertising.
        DEFAULT_ADV_INTERVAL       Default time between two advertising.
                                   messages.
        DEFAULT_CONNECTION_TIMEOUT Default timeout for connections between.
                                   two branches.
        DEFAULT_LOGGER_VERBOSITY   Default verbosity for newly created
                                   loggers.
        DEFAULT_LOG_TIME_FORMAT    Default format of the time string in log.
                                   entries.
        DEFAULT_LOG_FORMAT         Default format of a log entry.
        MAX_MESSAGE_SIZE           Maximum size of a message between branches.
    """
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
