from enum import IntEnum


class Verbosity(IntEnum):
    """Levels of how verbose logging output is.

    Attributes:
        FATAL   Fatal errors are error that requires a process restart.
        ERROR   Errors that the system can recover from.
        WARNING Warnings.
        INFO    Useful general information about the system state.
        DEBUG   Information for debugging.
        TRACE   Detailed debugging.
    """
    FATAL = 0
    ERROR = 1
    WARNING = 2
    INFO = 3
    DEBUG = 4
    TRACE = 5


class Stream(IntEnum):
    """Output streams for writing to the terminal.

    Attributes:
        STDOUT Standard output.
        STDERR Standard error output.
    """
    STDOUT = 0
    STDERR = 1
