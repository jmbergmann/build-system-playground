from enum import IntEnum


class Signals(IntEnum):
    """Signals.

    Attributes:
        NONE No signal
        INT  Interrupt (e.g. by receiving SIGINT or pressing STRG + C)
        TERM Termination request (e.g. by receiving SIGTERM)
        USR1 User-defined signal 1
        USR2 User-defined signal 2
        USR3 User-defined signal 3
        USR4 User-defined signal 4
        USR5 User-defined signal 5
        USR6 User-defined signal 6
        USR7 User-defined signal 7
        USR8 User-defined signal 8
        ALL  All signals
    """
    NONE = 0
    INT = (1 << 0)
    TERM = (1 << 1)
    USR1 = (1 << 24)
    USR2 = (1 << 25)
    USR3 = (1 << 26)
    USR4 = (1 << 27)
    USR5 = (1 << 28)
    USR6 = (1 << 29)
    USR7 = (1 << 30)
    USR8 = (1 << 31)
    ALL = INT | TERM | USR1 | USR2 | USR3 | USR4 | USR5 | USR6 | USR7 | USR8
