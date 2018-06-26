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
    """Entry point into a Yogi network.

    A branch represents an entry point into a YOGI network. It advertises itself
    via IP broadcasts/multicasts with its unique ID and information required for
    establishing a connection. If a branch detects other branches on the network,
    it connects to them via TCP to retrieve further information such as their
    name, description and network name. If the network names match, two branches
    attempt to authenticate with each other by securely comparing passwords.
    Once authentication succeeds and there is no other known branch with the same
    path then the branches can actively communicate as part of the Yogi network.

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
        pass

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
              "start_time":           "2018-04-23T18:25:43.511Z",
              "timeout":              3.0
            }

        Returns:
            Dictionary containing constant information about the branch.
        """
        pass # TODO: make sure to use inf instead of -1 for times and convert start time

    @property
    def uuid(self) -> str:
        """UUID of the branch."""
        return self.info.uuid

    @property
    def name(self) -> str:
        """Name of the branch."""
        return self.info.name

    @property
    def description(self) -> str:
        """Description of the branch."""
        return self.info.description

    @property
    def net_name(self) -> str:
        """Name of the network."""
        return self.info.net_name

    @property
    def path(self) -> str:
        """Path of the branch."""
        return self.info.path

    @property
    def hostname(self) -> str:
        """The machine's hostname."""
        return self.info.hostname

    @property
    def pid(self) -> int:
        """ID of the process."""
        return self.info.pid

    @property
    def advertising_address(self) -> str:
        """Advertising IP address."""
        return self.info.advertising_address

    @property
    def advertising_port(self) -> int:
        """Advertising port."""
        return self.info.advertising_port

    @property
    def advertising_interval(self) -> float:
        """Advertising interval."""
        return self.info.advertising_interval

    @property
    def tcp_server_address(self) -> str:
        """Address of the TCP server for incoming connections."""
        return self.info.tcp_server_address

    @property
    def tcp_server_port(self) -> int:
        """Listening port of the TCP server for incoming connections."""
        return self.info.tcp_server_port

    @property
    def start_time(self) -> datetime.datetime:
        """Time when the branch was started."""
        return self.info.start_time

    @property
    def timeout(self) -> float:
        """Connection timeout."""
        return self.info.timeout

