from .errors import *
from ctypes import *
from enum import Enum


class Verbosity(Enum):
    FATAL = 0
    ERROR = 1
    WARNING = 2
    INFO = 3
    DEBUG = 4
    TRACE = 5


class Stream(Enum):
    STDOUT = 0
    STDERR = 1
