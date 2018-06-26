from .object import Object
from .errors import Result, Failure, ErrorCode, api_result_handler
from .library import yogi
from .handler import Handler
from .context import Context

import datetime
from typing import Callable, Any
from ctypes import c_int, c_longlong, c_void_p, CFUNCTYPE, POINTER, byref


yogi.YOGI_TimerCreate.restype = api_result_handler
yogi.YOGI_TimerCreate.argtypes = [POINTER(c_void_p), c_void_p]

yogi.YOGI_TimerStart.restype = api_result_handler
yogi.YOGI_TimerStart.argtypes = [
    c_void_p, c_longlong, CFUNCTYPE(None, c_int, c_void_p), c_void_p]

yogi.YOGI_TimerCancel.restype = api_result_handler
yogi.YOGI_TimerCancel.argtypes = [c_void_p]


class Timer(Object):
    """Simple timer implementation."""

    def __init__(self, context: Context):
        """Creates the timer.

        Args:
            context: The context to use.
        """
        handle = c_void_p()
        yogi.YOGI_TimerCreate(byref(handle), context._handle)
        Object.__init__(self, handle, [context])

    def start(self, duration: float, fn: Callable[[Result], Any]) -> None:
        """Starts the timer in single shot mode.

        If the timer is already running, the timer will be canceled first, as
        if stop() were called explicitly.

        Args:
            duration: Time when the timer expires.
            fn:       Handler function to call after the given time passed.
        """
        t = -1 if duration == float('inf') else int(duration * 1e9)
        with Handler(yogi.YOGI_TimerStart.argtypes[2], fn) as handler:
            yogi.YOGI_TimerStart(self._handle, t, handler, None)

    def cancel(self) -> bool:
        """Cancels the given timer.

        Canceling a timer will result in the handler function registered via
        start() to be called with an cancelation error. Note that if the
        handler is already scheduled for executing, it will be called without
        an error.

        Returns:
            True if the timer was canceled successfully and False otherwise
            (i.e. the timner has not been started or it already expired).
        """
        try:
            yogi.YOGI_TimerCancel(self._handle)
        except Failure as failure:
            if failure.error_code is ErrorCode.TIMER_EXPIRED:
                return False
            else:
                raise

        return True
