from .object import Object
from .errors import Result, FailureException, Success, ErrorCode, \
    api_result_handler, error_code_to_result
from .library import yogi
from .handler import Handler
from .context import Context
from .timestamp import Timestamp

import json
from enum import IntEnum
from uuid import UUID
from typing import Callable, Any, Optional, Dict
from ctypes import c_int, c_longlong, c_void_p, c_char_p, CFUNCTYPE, \
    POINTER, byref, create_string_buffer, sizeof


class BranchEvents(IntEnum):
    """Events that can be observed on a branch.

    Attributes:
        NONE              No events.
        BRANCH_DISCOVERED A new branch has been discovered.
        BRANCH_QUERIED    Querying a new branch for information finished.
        CONNECT_FINISHED  Connecting to a branch finished.
        CONNECTION_LOST   The connection to a branch was lost.
        ALL               Combination of all flags.
    """
    NONE = 0
    BRANCH_DISCOVERED = (1 << 0)
    BRANCH_QUERIED = (1 << 1)
    CONNECT_FINISHED = (1 << 2)
    CONNECTION_LOST = (1 << 3)
    ALL = BRANCH_DISCOVERED | BRANCH_QUERIED | CONNECT_FINISHED \
        | CONNECTION_LOST


def convert_info_fields(info):
    if info["advertising_interval"] == -1:
        info["advertising_interval"] = float("inf")

    if info["timeout"] == -1:
        info["timeout"] = float("inf")

    info["uuid"] = UUID(info["uuid"])
    info["start_time"] = Timestamp.parse(info["start_time"])


class BranchInfo:
    """Information about a branch."""

    def __init__(self, info_string: str):
        self._info = json.loads(info_string)
        convert_info_fields(self._info)

    def __str__(self) -> str:
        return str(self._info)

    @property
    def uuid(self) -> UUID:
        """UUID of the branch."""
        return self._info["uuid"]

    @property
    def name(self) -> str:
        """Name of the branch."""
        return self._info["name"]

    @property
    def description(self) -> str:
        """Description of the branch."""
        return self._info["description"]

    @property
    def network_name(self) -> str:
        """Name of the network."""
        return self._info["network_name"]

    @property
    def path(self) -> str:
        """Path of the branch."""
        return self._info["path"]

    @property
    def hostname(self) -> str:
        """The machine's hostname."""
        return self._info["hostname"]

    @property
    def pid(self) -> int:
        """ID of the process."""
        return self._info["pid"]

    @property
    def advertising_interval(self) -> float:
        """Advertising interval."""
        return self._info["advertising_interval"]

    @property
    def tcp_server_address(self) -> str:
        """Address of the TCP server for incoming connections."""
        return self._info["tcp_server_address"]

    @property
    def tcp_server_port(self) -> int:
        """Listening port of the TCP server for incoming connections."""
        return self._info["tcp_server_port"]

    @property
    def start_time(self) -> Timestamp:
        """Time when the branch was started."""
        return self._info["start_time"]

    @property
    def timeout(self) -> float:
        """Connection timeout."""
        return self._info["timeout"]


class RemoteBranchInfo(BranchInfo):
    """Information about a remote branch."""

    def __init__(self, info_string: str):
        BranchInfo.__init__(self, info_string)


class LocalBranchInfo(BranchInfo):
    """Information about a local branch."""

    def __init__(self, info_string: str):
        BranchInfo.__init__(self, info_string)

    @property
    def advertising_address(self) -> str:
        """Advertising IP address."""
        return self._info["advertising_address"]

    @property
    def advertising_port(self) -> int:
        """Advertising port."""
        return self._info["advertising_port"]


class BranchEventInfo:
    """Information associated with a branch event."""

    def __init__(self, info_string: str):
        self._info = json.loads(info_string)
        convert_info_fields(self._info)

    def __str__(self) -> str:
        return str(self._info)

    @property
    def uuid(self) -> UUID:
        """UUID of the branch."""
        return self._info["uuid"]


class BranchDiscoveredEventInfo(BranchEventInfo):
    """Information associated with the BRANCH_DISCOVERED event."""

    def __init__(self, info_string: str):
        BranchEventInfo.__init__(self, info_string)

    @property
    def tcp_server_address(self) -> str:
        """Address of the TCP server for incoming connections."""
        return self._info["tcp_server_address"]

    @property
    def tcp_server_port(self) -> int:
        """Listening port of the TCP server for incoming connections."""
        return self._info["tcp_server_port"]


class BranchQueriedEventInfo(BranchEventInfo, RemoteBranchInfo):
    """Information associated with the BRANCH_QUERIED event."""

    def __init__(self, info_string: str):
        BranchEventInfo.__init__(self, info_string)
        # No need to call RemoteBranchInfo's constructor here


class ConnectFinishedEventInfo(BranchEventInfo):
    """Information associated with the CONNECT_FINISHED event."""

    def __init__(self, info_string: str):
        BranchEventInfo.__init__(self, info_string)


class ConnectionLostEventInfo(BranchEventInfo):
    """Information associated with the CONNECTION_LOST event."""

    def __init__(self, info_string: str):
        BranchEventInfo.__init__(self, info_string)


yogi.YOGI_BranchCreate.restype = api_result_handler
yogi.YOGI_BranchCreate.argtypes = [
    POINTER(c_void_p), c_void_p, c_char_p, c_char_p, c_char_p, c_char_p,
    c_char_p, c_char_p, c_int, c_longlong, c_longlong]

yogi.YOGI_BranchGetInfo.restype = api_result_handler
yogi.YOGI_BranchGetInfo.argtypes = [c_void_p, c_void_p, c_char_p, c_int]

yogi.YOGI_BranchGetConnectedBranches.restype = api_result_handler
yogi.YOGI_BranchGetConnectedBranches.argtypes = [
    c_void_p, c_void_p, c_char_p, c_int, CFUNCTYPE(None, c_int, c_void_p),
    c_void_p]

yogi.YOGI_BranchAwaitEvent.restype = api_result_handler
yogi.YOGI_BranchAwaitEvent.argtypes = [
    c_void_p, c_int, c_void_p, c_char_p, c_int,
    CFUNCTYPE(None, c_int, c_int, c_int, c_void_p), c_void_p]

yogi.YOGI_BranchCancelAwaitEvent.restype = api_result_handler
yogi.YOGI_BranchCancelAwaitEvent.argtypes = [c_void_p]


class Branch(Object):
    """Entry point into a Yogi network.

    A branch represents an entry point into a YOGI network. It advertises
    itself via IP broadcasts/multicasts with its unique ID and information
    required for establishing a connection. If a branch detects other branches
    on the network, it connects to them via TCP to retrieve further
    information such as their name, description and network name. If the
    network names match, two branches attempt to authenticate with each other
    by securely comparing passwords. Once authentication succeeds and there is
    no other known branch with the same path then the branches can actively
    communicate as part of the Yogi network.

    Note: Even though the authentication process via passwords is done in a
          secure manner, any further communication is done in plain text.
    """

    def __init__(self, context: Context, name: str = None,
                 description: str = None, netname: str = None,
                 password: str = None, path: str = None,
                 advaddr: str = None, advport: int = None,
                 advint: float = None, timeout: float = None):
        """Creates the branch.

        Advertising and establishing connections can be limited to certain
        network interfaces via the interface parameter. The default is to use
        all available interfaces.

        Setting the advint parameter to infinity prevents the branch from
        actively participating in the Yogi network, i.e. the branch will not
        advertise itself and it will not authenticate in order to join a
        network. However, the branch will temporarily connect to other
        branches in order to obtain more detailed information such as name,
        description, network name and so on. This is useful for obtaining
        information about active branches without actually becoming part of
        the Yogi network.

        Args:
            context:     The context to use.
            name:        Name of the branch (by default, the format
                         PID@hostname with PID being the process ID will be
                         used).
            description: Description of the branch.
            netname:     Name of the network to join (by default, the
                         machine's hostname will be used as the network name).
            password:    Password for the network.
            path:        Path of the branch in the network (by default, the
                         format /name where name is the branch's name will
                         be used). Must start with a slash.
            advaddr:     Multicast address to use; e.g. 239.255.0.1 for IPv4
                         or ff31::8000:1234 for IPv6.
            advport:     Advertising port.
            advint:      Advertising interval (must be at least 0.001).
            timeout:     Maximum time of inactivity before a remote branch is
                         considered to be dead (must be at least 0.001).
        """
        def conv_string(s):
            if s is None:
                return None
            else:
                return s.encode("utf-8")

        def conv_duration(duration):
            if duration is None:
                return 0
            elif duration == float("inf"):
                return -1
            else:
                return int(duration * 1e9)

        handle = c_void_p()
        yogi.YOGI_BranchCreate(byref(handle), context._handle,
                               conv_string(name), conv_string(description),
                               conv_string(netname), conv_string(password),
                               conv_string(path), conv_string(advaddr),
                               advport or 0, conv_duration(advint),
                               conv_duration(timeout))
        Object.__init__(self, handle, [context])
        self._info = self.__get_info()

    @property
    def info(self) -> LocalBranchInfo:
        """Information about the local branch.

        Returns:
            Object containing information about the branch.
        """
        return self._info

    @property
    def uuid(self) -> UUID:
        """UUID of the branch."""
        return self._info.uuid

    @property
    def name(self) -> str:
        """Name of the branch."""
        return self._info.name

    @property
    def description(self) -> str:
        """Description of the branch."""
        return self._info.description

    @property
    def net_name(self) -> str:
        """Name of the network."""
        return self._info.network_name

    @property
    def path(self) -> str:
        """Path of the branch."""
        return self._info.path

    @property
    def hostname(self) -> str:
        """The machine's hostname."""
        return self._info.hostname

    @property
    def pid(self) -> int:
        """ID of the process."""
        return self._info.pid

    @property
    def advertising_address(self) -> str:
        """Advertising IP address."""
        return self._info.advertising_address

    @property
    def advertising_port(self) -> int:
        """Advertising port."""
        return self._info.advertising_port

    @property
    def advertising_interval(self) -> float:
        """Advertising interval."""
        return self._info.advertising_interval

    @property
    def tcp_server_address(self) -> str:
        """Address of the TCP server for incoming connections."""
        return self._info.tcp_server_address

    @property
    def tcp_server_port(self) -> int:
        """Listening port of the TCP server for incoming connections."""
        return self._info.tcp_server_port

    @property
    def start_time(self) -> Timestamp:
        """Time when the branch was started."""
        return self._info.start_time

    @property
    def timeout(self) -> float:
        """Connection timeout."""
        return self._info.timeout

    def get_connected_branches(self) -> Dict[str, RemoteBranchInfo]:
        """Retrieves information about all connected remote branches.

        Returns:
            Dictionary mapping the UUID of each connected remote branch to an
            object containing detailed information.
        """
        s = create_string_buffer(1024)
        strings = []

        def append_string(res, userarg):
            strings.append(s.value.decode("utf-8"))

        c_append_string = yogi.YOGI_BranchGetConnectedBranches.argtypes[4](
            append_string)

        while True:
            try:
                yogi.YOGI_BranchGetConnectedBranches(
                    self._handle, None, s, sizeof(s), c_append_string, None)
                break
            except FailureException as e:
                if e.failure.error_code is ErrorCode.BUFFER_TOO_SMALL:
                    strings = []
                    s = create_string_buffer(sizeof(s) * 2)
                else:
                    raise

        branches = {}
        for string in strings:
            info = RemoteBranchInfo(string)
            branches[info.uuid] = info

        return branches

    def await_event(self, events: BranchEvents,
                    fn: Callable[[Result, BranchEvents, Result,
                                  Optional[BranchEventInfo]], Any],
                    buffer_size: int = 1024) -> None:
        """Waits for a branch event to occur.

        This function will register the handler fn to be executed once one of
        the given branch events occurs. The handler's parameters are, from
        left to right, the result associated with the wait operation, the
        event that occurred, the result associated with the event, and a
        dictionary containing event details.

        If this function is called while a previous wait operation is still
        active then the previous operation will be canceled, i.e. the handler
        fn for the previous operation will be called with a cancellation
        error.

        If successful, the event information passed to the handler function fn
        contains at least the UUID of the remote branch.

        In case that the internal buffer for reading the event information is
        too small, fn will be called with the corresponding error and the
        event information is lost. You can set the size of this buffer via the
        bufferSize parameter.

        Args:
            events:      Events to observe.
            fn:          Handler function to call.
            buffer_size: Size of the internal buffer for reading the event
                         information.
        """
        s = create_string_buffer(buffer_size)

        def wrapped_fn(res, event, evres):
            info = None
            if res == Success():
                string = s.value.decode("utf-8")
                if event == BranchEvents.BRANCH_DISCOVERED:
                    info = BranchDiscoveredEventInfo(string)
                elif event == BranchEvents.BRANCH_QUERIED:
                    info = BranchQueriedEventInfo(string)
                elif event == BranchEvents.CONNECT_FINISHED:
                    info = ConnectFinishedEventInfo(string)
                elif event == BranchEvents.CONNECTION_LOST:
                    info = ConnectionLostEventInfo(string)
                else:
                    info = BranchEventInfo(string)

            fn(res, BranchEvents(event), error_code_to_result(evres), info)

        with Handler(yogi.YOGI_BranchAwaitEvent.argtypes[5], wrapped_fn
                     )as handler:
            yogi.YOGI_BranchAwaitEvent(self._handle, events, None, s,
                                       sizeof(s), handler, None)

    def cancel_await_event(self) -> None:
        """Cancels waiting for a branch event.

        Calling this function will cause the handler registerd via
        await_event() to be called with a cancellation error.
        """
        yogi.YOGI_BranchCancelAwaitEvent(self._handle)

    def __get_info(self) -> LocalBranchInfo:
        s = create_string_buffer(1024)
        while True:
            try:
                yogi.YOGI_BranchGetInfo(self._handle, None, s, sizeof(s))
                break
            except FailureException as e:
                if e.failure.error_code is ErrorCode.BUFFER_TOO_SMALL:
                    s = create_string_buffer(sizeof(s) * 2)
                else:
                    raise

        return LocalBranchInfo(s.value.decode("utf-8"))
