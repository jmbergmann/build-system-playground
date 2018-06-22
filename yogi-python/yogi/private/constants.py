from .errors import *
from ctypes import *
from .logging import Verbosity
from .class_property import class_property


yogi.YOGI_GetConstant.restype = api_result_handler
yogi.YOGI_GetConstant.argtypes = [c_void_p, c_int]

def get_constant(api_id, api_type):
    c = api_type();
    yogi.YOGI_GetConstant(byref(c), api_id)
    return c.value.decode() if api_type is c_char_p else c.value

class Constants:
    @class_property
    def VERSION_NUMBER(cls) -> str:
        return get_constant(1, c_char_p)

    @class_property
    def VERSION_MAJOR(cls) -> int:
        return get_constant(2, c_int)

    @class_property
    def VERSION_MINOR(cls) -> int:
        return get_constant(3, c_int)

    @class_property
    def VERSION_PATCH(cls) -> int:
        return get_constant(4, c_int)

    @class_property
    def DEFAULT_ADV_ADDRESS(cls) -> str:
        return get_constant(5, c_char_p)

    @class_property
    def DEFAULT_ADV_PORT(cls) -> int:
        return get_constant(6, c_int)

    @class_property
    def DEFAULT_ADV_INTERVAL(cls) -> int:
        return get_constant(7, c_longlong)

    @class_property
    def DEFAULT_CONNECTION_TIMEOUT(cls) -> int:
        return get_constant(8, c_longlong)

    @class_property
    def DEFAULT_LOGGER_VERBOSITY(cls) -> Verbosity:
        return Verbosity(get_constant(9, c_int))

    @class_property
    def DEFAULT_LOG_TIME_FORMAT(cls) -> str:
        return get_constant(10, c_char_p)

    @class_property
    def DEFAULT_LOG_FORMAT(cls) -> str:
        return get_constant(11, c_char_p)

    @class_property
    def MAX_MESSAGE_SIZE(cls) -> int:
        return get_constant(12, c_int)
