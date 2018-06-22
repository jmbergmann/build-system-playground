from .errors import *
from ctypes import *
import datetime
import time


yogi.YOGI_GetCurrentTime.restype = api_result_handler
yogi.YOGI_GetCurrentTime.argtypes = [POINTER(c_longlong)]

def get_current_time() -> datetime.datetime:
    timestamp = c_longlong()
    yogi.YOGI_GetCurrentTime(byref(timestamp))
    t = datetime.datetime(1970, 1, 1)
    t += datetime.timedelta(microseconds=timestamp.value / 1000)
    t -= datetime.timedelta(seconds=time.timezone)
    return t  # TODO: there is probably a better way...
