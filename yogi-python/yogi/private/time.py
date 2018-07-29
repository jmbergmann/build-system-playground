from .errors import api_result_handler
from .library import yogi

import datetime
from ctypes import c_longlong, c_char_p, c_int, byref, POINTER, \
    create_string_buffer, sizeof


yogi.YOGI_GetCurrentTime.restype = api_result_handler
yogi.YOGI_GetCurrentTime.argtypes = [POINTER(c_longlong)]

yogi.YOGI_FormatTime.restype = api_result_handler
yogi.YOGI_FormatTime.argtypes = [c_longlong, c_char_p, c_int, c_char_p]

yogi.YOGI_ParseTime.restype = api_result_handler
yogi.YOGI_ParseTime.argtypes = [POINTER(c_longlong), c_char_p, c_char_p]


def timestamp_to_datetime(timestamp: int) -> datetime.datetime:
    epoch = datetime.datetime(1970, 1, 1, tzinfo=datetime.timezone.utc)
    delta = datetime.timedelta(microseconds=timestamp / 1000)
    return epoch + delta


def string_to_datetime(s: str) -> datetime.datetime:
    return datetime.datetime.strptime(s + "+0000", "%Y-%m-%dT%H:%M:%S.%fZ%z")


def get_current_time() -> datetime.datetime:
    """Returns the current time.

    Returns:
        Aware datetime object representing the current time.
    """
    timestamp = c_longlong()
    yogi.YOGI_GetCurrentTime(byref(timestamp))
    return timestamp_to_datetime(timestamp.value)


def format_time(time: datetime.datetime, fmt: str = None) -> str:
    """Converts a timestamp into a string.

    The timefmt parameter describes the textual format of the conversion. The
    following placeholders are supported:
        %Y: Four digit year.
        %m: Month name as a decimal 01 to 12.
        %d: Day of the month as decimal 01 to 31.
        %F: Equivalent to %Y-%m-%d (the ISO 8601 date format).
        %H: The hour as a decimal number using a 24-hour clock (00 to 23).
        %M: The minute as a decimal 00 to 59.
        %S: Seconds as a decimal 00 to 59.
        %T: Equivalent to %H:%M:%S (the ISO 8601 time format).
        %3: Milliseconds as decimal number 000 to 999.
        %6: Microseconds as decimal number 000 to 999.
        %9: Nanoseconds as decimal number 000 to 999.

    By default, the timestamp will be formatted in the format
    "2009-02-11T12:53:09.123Z".

    Args:
        time: The timestamp.
        fmt:  Format of the time string.

    Returns:
        The timestamp as a string.
    """
    timestamp = c_longlong(int(time.timestamp() * 1e9))
    fmt = None if fmt is None else fmt.encode("utf-8")
    s = create_string_buffer(128)
    yogi.YOGI_FormatTime(timestamp, s, sizeof(s), fmt)
    return s.value.decode("utf-8")

def parse_time(s: str, fmt: str = None) -> datetime.datetime:
    """Converts a string into a timestamp.

    The timefmt parameter describes the textual format of the conversion. The
    following placeholders are supported:
        %Y: Four digit year.
        %m: Month name as a decimal 01 to 12.
        %d: Day of the month as decimal 01 to 31.
        %F: Equivalent to %Y-%m-%d (the ISO 8601 date format).
        %H: The hour as a decimal number using a 24-hour clock (00 to 23).
        %M: The minute as a decimal 00 to 59.
        %S: Seconds as a decimal 00 to 59.
        %T: Equivalent to %H:%M:%S (the ISO 8601 time format).
        %3: Milliseconds as decimal number 000 to 999.
        %6: Microseconds as decimal number 000 to 999.
        %9: Nanoseconds as decimal number 000 to 999.

    By default, the timestamp will be parsed in the format
    "2009-02-11T12:53:09.123Z".

    Args:
        s:   String to parse.
        fmt: Format of the time string.

    Returns:
        The timestamp.
    """
    timestamp = c_longlong()
    s = s.encode("utf-8")
    fmt = None if fmt is None else fmt.encode("utf-8")
    yogi.YOGI_ParseTime(byref(timestamp), s, fmt)
    return timestamp_to_datetime(timestamp.value)
