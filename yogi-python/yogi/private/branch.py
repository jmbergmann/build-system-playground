from .object import Object
from .errors import Result, Failure, Success, ErrorCode, api_result_handler, \
    error_code_to_result
from .library import yogi
from .handler import Handler
from .context import Context
from .time import string_to_datetime

import json
import datetime
from enum import IntEnum
from typing import Callable, Any, Optional
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

    info["start_time"] = string_to_datetime(
        info["start_time"])


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

    def __init__(self, context: Context, name: str, description: str = None,
                 netname: str = None, password: str = None, path: str = None,
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
                         be used).
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
        self._info = None

    @property
    def info(self) -> dict:
        """Information about the local branch.

        The returned information has the following structure:
            {
              "uuid":                 "6ba7b810-9dad-11d1-80b4-00c04fd430c8",
              "name":                 "Fan Controller",
              "description":          "Controls a fan via PWM",
              "net_name":             "Hardware Control",
              "path":                 "/Cooling System/Fan Controller",
              "hostname":             "beaglebone",
              "pid":                  4124,
              "advertising_address":  "ff31::8000:2439",
              "advertising_port":     13531,
              "advertising_interval": 1.0,
              "tcp_server_address":   "::",
              "tcp_server_port":      53332,
              "start_time":           <instance of datetime.datetime>,
              "timeout":              3.0
            }

        Returns:
            Dictionary containing constant information about the branch.
        """
        if not self._info:
            s = create_string_buffer(1024)
            while True:
                try:
                    yogi.YOGI_BranchGetInfo(self._handle, None, s, sizeof(s))
                    break
                except Failure as failure:
                    if failure.error_code is ErrorCode.BUFFER_TOO_SMALL:
                        s = create_string_buffer(sizeof(s) * 2)
                    else:
                        raise

            self._info = json.loads(s.value.decode("utf-8"))
            convert_info_fields(self._info)

        return self._info

    @property
    def uuid(self) -> str:
        """UUID of the branch."""
        return self.info["uuid"]

    @property
    def name(self) -> str:
        """Name of the branch."""
        return self.info["name"]

    @property
    def description(self) -> str:
        """Description of the branch."""
        return self.info["description"]

    @property
    def net_name(self) -> str:
        """Name of the network."""
        return self.info["net_name"]

    @property
    def path(self) -> str:
        """Path of the branch."""
        return self.info["path"]

    @property
    def hostname(self) -> str:
        """The machine's hostname."""
        return self.info["hostname"]

    @property
    def pid(self) -> int:
        """ID of the process."""
        return self.info["pid"]

    @property
    def advertising_address(self) -> str:
        """Advertising IP address."""
        return self.info["advertising_address"]

    @property
    def advertising_port(self) -> int:
        """Advertising port."""
        return self.info["advertising_port"]

    @property
    def advertising_interval(self) -> float:
        """Advertising interval."""
        return self.info["advertising_interval"]

    @property
    def tcp_server_address(self) -> str:
        """Address of the TCP server for incoming connections."""
        return self.info["tcp_server_address"]

    @property
    def tcp_server_port(self) -> int:
        """Listening port of the TCP server for incoming connections."""
        return self.info["tcp_server_port"]

    @property
    def start_time(self) -> datetime.datetime:
        """Time when the branch was started."""
        return self.info["start_time"]

    @property
    def timeout(self) -> float:
        """Connection timeout."""
        return self.info["timeout"]

    def get_connected_branches(self) -> dict:
        """Retrieves information about all connected remote branches.

        This function returns a dictionary where each is the UUID of the
        connected remote branch and the value is another dictionary with the
        following structure:
            {
              "uuid":                 "123e4567-e89b-12d3-a456-426655440000",
              "name":                 "Pump Safety Logic",
              "description":          "Monitors the pump for safety",
              "net_name":             "Hardware Control",
              "path":                 "/Cooling System/Pump/Safety",
              "hostname":             "beaglebone",
              "pid":                  3321,
              "tcp_server_address":   "fe80::f086:b106:2c1b:c45",
              "tcp_server_port":      43384,
              "start_time":           <instance of datetime.datetime>,
              "timeout":              3.0,
              "advertising_interval": 1.0
            }

        Returns:
            Dictionary mapping the UUID of each connected remote branch to
            another dictionary with detailed information.
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
            except Failure as failure:
                if failure.error_code is ErrorCode.BUFFER_TOO_SMALL:
                    strings = []
                    s = create_string_buffer(sizeof(s) * 2)
                else:
                    raise

        branches = {}
        for string in strings:
            info = json.loads(string)
            convert_info_fields(info)
            branches[info["uuid"]] = info

        return branches

    def await_event(self, events: BranchEvents,
                    fn: Callable[[Result, BranchEvents, Result, Optional[dict]
                                  ], Any]) -> None:
        """Wait for a branch event to occur.

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
        contains at least the UUID of the remote branch. For newly discovered
        branches, both address and port of the remote TCP server are included
        and when querying a remote branch succeeds, all of the available
        branch information is included:
            {
              "uuid":                 "123e4567-e89b-12d3-a456-426655440000",
              "name":                 "Pump Safety Logic",
              "description":          "Monitors the pump for safety",
              "net_name":             "Hardware Control",
              "path":                 "/Cooling System/Pump/Safety",
              "hostname":             "beaglebone",
              "pid":                  3321,
              "tcp_server_address":   "fe80::f086:b106:2c1b:c45",
              "tcp_server_port":      43384,
              "start_time":           <instance of datetime.datetime>,
              "timeout":              3.0,
              "advertising_interval": 1.0
            }

        Args:
            events: Events to observe.
            fn:     Handler to call.
        """
        s = create_string_buffer(10240)

        def wrapped_fn(res, event, evres):
            info = None
            if res == Success():
                info = json.loads(s.value.decode("utf-8"))
                convert_info_fields(info)

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
