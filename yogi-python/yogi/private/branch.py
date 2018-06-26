from .object import Object
from .errors import api_result_handler
from .library import yogi
from .handler import Handler
from .context import Context

import datetime
from enum import IntEnum
from typing import Callable, Any
from ctypes import c_int, c_longlong, c_void_p, CFUNCTYPE, POINTER, byref


class BranchEvents(IntEnum):
    """Events that can be observed on a branch.

    Attributes:
        NONE              No events
        BRANCH_DISCOVERED A new branch has been discovered.
        BRANCH_QUERIED    Querying a new branch for information finished.
        CONNECT_FINISHED  Connecting to a branch finished.
        CONNECTION_LOST   The connection to a branch was lost.
    """
    NONE = 0
    BRANCH_DISCOVERED = (1 << 0)
    BRANCH_QUERIED = (1 << 1)
    CONNECT_FINISHED = (1 << 2)
    CONNECTION_LOST = (1 << 3)
    ALL = BRANCH_DISCOVERED | BRANCH_QUERIED | CONNECT_FINISHED \
        | CONNECTION_LOST


class Branch(Object):
    def __init__(self, context: Context, name: str, description: str, netname: str, password: str, path: str, advaddr: str, advport: int, advint: datetime.timedelta, timeout: datetime.timedelta):
        pass
