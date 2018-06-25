from .errors import api_result_handler
from .library import yogi

import datetime
from ctypes import c_longlong, byref, POINTER


yogi.YOGI_GetCurrentTime.restype = api_result_handler
yogi.YOGI_GetCurrentTime.argtypes = [POINTER(c_longlong)]


def timestamp_to_datetime(timestamp: int) -> datetime.datetime:
    epoch = datetime.datetime(1970, 1, 1, tzinfo=datetime.timezone.utc)
    delta = datetime.timedelta(microseconds=timestamp / 1000)
    return epoch + delta


def get_current_time() -> datetime.datetime:
    """Returns the current time.

    Returns:
        Aware datetime object representing the current time.
    """
    timestamp = c_longlong()
    yogi.YOGI_GetCurrentTime(byref(timestamp))
    return timestamp_to_datetime(timestamp.value)
