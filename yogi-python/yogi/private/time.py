from .errors import api_result_handler
from .library import yogi

import datetime
import time
from ctypes import c_longlong, byref, POINTER


yogi.YOGI_GetCurrentTime.restype = api_result_handler
yogi.YOGI_GetCurrentTime.argtypes = [POINTER(c_longlong)]


def get_current_time() -> datetime.datetime:
    timestamp = c_longlong()
    yogi.YOGI_GetCurrentTime(byref(timestamp))
    epoch = datetime.datetime(1970, 1, 1)
    return epoch + datetime.timedelta(microseconds=timestamp.value / 1000,
                                      seconds=-time.timezone)
