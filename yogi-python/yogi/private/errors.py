from .library import yogi

from ctypes import c_char_p, c_int


yogi.YOGI_GetErrorString.restype = c_char_p
yogi.YOGI_GetErrorString.argtypes = [c_int]


class Result:
    """Represents a result of an operation.

    This is a wrapper around the result code returned by the function from the
    Yogi Core library. A result is represented by a number which is >= 0 in
    case of success and < 0 in case of a failure.
    """
    def __init__(self, value: int):
        """Constructs the result.

        Args:
            value: Number as returned by the Yogi Core library functions.
        """
        self._value = value

    @property
    def value(self) -> int:
        """The number as returned by the Yogi Core library functions.
        """
        return self._value

    def __bool__(self) -> bool:
        return self._value >= 0

    def __eq__(self, other) -> bool:
        return isinstance(other, Result) and self._value == other.value

    def __ne__(self, other) -> bool:
        return not (self == other)

    def __str__(self) -> str:
        return yogi.YOGI_GetErrorString(self._value).decode()

    def __hash__(self)-> int:
        return self._value


class Failure(Exception, Result):
    """Represents the failure of an operation.
    """
    def __init__(self, value: int):
        assert value < 0
        Result.__init__(self, value)

    def __str__(self) -> str:
        return Result.__str__(self)


class Success(Result):
    """Represents the success of an operation.
    """
    def __init__(self, value: int = 0):
        assert value >= 0
        Result.__init__(self, value)


def api_result_handler(result: int) -> Result:
    if result < 0:
        raise Failure(result)
    else:
        return Success(result)


def error_code_to_result(result: int) -> Result:
    if result < 0:
        return Failure(result)
    else:
        return Success(result)
