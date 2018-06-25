from .library import yogi

from ctypes import c_char_p, c_int
from enum import IntEnum


class ErrorCode(IntEnum):
    """Error codes returned by functions from the Yogi Core library."""
    OK = 0
    UNKNOWN = -1
    OBJECT_STILL_USED = -2
    BAD_ALLOC = -3
    INVALID_PARAM = -4
    INVALID_HANDLE = -5
    WRONG_OBJECT_TYPE = -6
    CANCELED = -7
    BUSY = -8
    TIMEOUT = -9
    TIMER_EXPIRED = -10
    BUFFER_TOO_SMALL = -11
    OPEN_SOCKET_FAILED = -12
    BIND_SOCKET_FAILED = -13
    LISTEN_SOCKET_FAILED = -14
    SET_SOCKET_OPTION_FAILED = -15
    INVALID_REGEX = -16
    OPEN_FILE_FAILED = -17
    RW_SOCKET_FAILED = -18
    CONNECT_SOCKET_FAILED = -19
    INVALID_MAGIC_PREFIX = -20
    INCOMPATIBLE_VERSION = -21
    DESERIALIZE_MSG_FAILED = -22
    ACCEPT_SOCKET_FAILED = -23
    LOOPBACK_CONNECTION = -24
    PASSWORD_MISMATCH = -25
    NET_NAME_MISMATCH = -26
    DUPLICATE_BRANCH_NAME = -27
    DUPLICATE_BRANCH_PATH = -28
    MESSAGE_TOO_LARGE = -29
    PARSING_CMDLINE_FAILED = -30
    PARSING_JSON_FAILED = -31
    PARSING_FILE_FAILED = -32
    CONFIG_NOT_VALID = -33
    HELP_REQUESTED = -34
    WRITE_TO_FILE_FAILED = -35
    UNDEFINED_VARIABLES = -36
    NO_VARIABLE_SUPPORT = -37
    VARIABLE_USED_IN_KEY = -38


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
        """The number as returned by the Yogi Core library functions."""
        return self._value

    @property
    def error_code(self) -> ErrorCode:
        """Error code associated with this result."""
        return ErrorCode(self._value) if self._value < 0 else ErrorCode.OK

    def __bool__(self) -> bool:
        return self._value >= 0

    def __eq__(self, other) -> bool:
        return isinstance(other, Result) and self._value == other.value

    def __ne__(self, other) -> bool:
        return not (self == other)

    def __str__(self) -> str:
        return yogi.YOGI_GetErrorString(self._value).decode("utf-8")

    def __hash__(self)-> int:
        return self._value


class Failure(Exception, Result):
    """Represents the failure of an operation."""

    def __init__(self, value: int):
        assert value < 0
        Result.__init__(self, value)

    def __str__(self) -> str:
        return Result.__str__(self)


class DescriptiveFailure(Failure):
    """A failure of an operation which includes a description."""

    def __init__(self, value: int, description: str):
        Failure.__init__(self, value)
        self._description = description

    @property
    def description(self) -> str:
        """More detailed information of the error."""
        return self._description

    def __str__(self) -> str:
        return Failure.__str__(self) + ". Description: " + self._description


class Success(Result):
    """Represents the success of an operation."""

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
