from .errors import api_result_handler
from .library import yogi
from .timestamp import Timestamp

import datetime
from ctypes import c_longlong, c_char_p, c_int, byref, POINTER, \
    create_string_buffer, sizeof


def get_current_time() -> Timestamp:
    """Returns the current time.

    Returns:
        Aware datetime object representing the current time.
    """
    return Timestamp.now()
