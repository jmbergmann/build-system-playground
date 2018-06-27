from .object import Object
from .errors import DescriptiveFailure, Failure, ErrorCode, \
    api_result_handler
from .library import yogi

from enum import IntEnum
import json
from typing import List, Union, Optional
from ctypes import c_int, c_char_p, c_char, c_void_p, POINTER, byref, \
    create_string_buffer, sizeof


class ConfigurationFlags(IntEnum):
    """Flags used to change a configuration object's behaviour.

    Attributes:
        NONE              No flags.
        DISABLE_VARIABLES Disables support for variables in the configuration.
        MUTABLE_CMDLINE   Makes configuration options given directly on the
                          command line overridable.
    """
    NONE = 0
    DISABLE_VARIABLES = (1 << 0)
    MUTABLE_CMDLINE = (1 << 1)


class CommandLineOptions(IntEnum):
    """Flags used to adjust how command line options are parsed.

    Attributes:
        NONE               No options.
        LOGGING            Include logging configuration for file logging.
        BRANCH_NAME        Include branch name configuration.
        BRANCH_DESCRIPTION Include branch description configuration.
        BRANCH_NETWORK     Include network name configuration.
        BRANCH_PASSWORD    Include network password configuration.
        BRANCH_PATH        Include branch path configuration.
        BRANCH_ADVADDR     Include branch advertising address configuration.
        BRANCH_ADVPORT     Include branch advertising port configuration.
        BRANCH_ADVINT      Include branch advertising interval configuration.
        BRANCH_TIMEOUT     Include branch timeout configuration.
        FILES              Parse configuration files given on the command
                           line.
        FILES_REQUIRED     Same as YOGI_CLO_FILES but at least one
                           configuration must be given.
        OVERRIDES          Allow overriding arbitrary configuration sections.
        VARIABLES          Allow setting variables via a dedicated switch.
        BRANCH_ALL         Combination of all branch flags.
        ALL                Combination of all flags.
    """
    NONE = 0
    LOGGING = (1 << 0)
    BRANCH_NAME = (1 << 1)
    BRANCH_DESCRIPTION = (1 << 2)
    BRANCH_NETWORK = (1 << 3)
    BRANCH_PASSWORD = (1 << 4)
    BRANCH_PATH = (1 << 5)
    BRANCH_ADVADDR = (1 << 6)
    BRANCH_ADVPORT = (1 << 7)
    BRANCH_ADVINT = (1 << 8)
    BRANCH_TIMEOUT = (1 << 9)
    FILES = (1 << 10)
    FILES_REQUIRED = (1 << 11)
    OVERRIDES = (1 << 12)
    VARIABLES = (1 << 13)
    BRANCH_ALL = BRANCH_NAME | BRANCH_DESCRIPTION | BRANCH_NETWORK \
        | BRANCH_PASSWORD | BRANCH_PATH | BRANCH_ADVADDR \
        | BRANCH_ADVPORT | BRANCH_ADVINT | BRANCH_TIMEOUT
    ALL = LOGGING | BRANCH_ALL | FILES | FILES_REQUIRED | OVERRIDES \
        | VARIABLES


yogi.YOGI_ConfigurationCreate.restype = api_result_handler
yogi.YOGI_ConfigurationCreate.argtypes = [POINTER(c_void_p), c_int]

yogi.YOGI_ConfigurationUpdateFromCommandLine.restype = api_result_handler
yogi.YOGI_ConfigurationUpdateFromCommandLine.argtypes = [
    c_void_p, c_int, POINTER(POINTER(c_char)), c_int, c_char_p, c_int]

yogi.YOGI_ConfigurationUpdateFromJson.restype = api_result_handler
yogi.YOGI_ConfigurationUpdateFromJson.argtypes = [c_void_p, c_char_p,
                                                  c_char_p, c_int]

yogi.YOGI_ConfigurationUpdateFromFile.restype = api_result_handler
yogi.YOGI_ConfigurationUpdateFromFile.argtypes = [c_void_p, c_char_p,
                                                  c_char_p, c_int]

yogi.YOGI_ConfigurationDump.restype = api_result_handler
yogi.YOGI_ConfigurationDump.argtypes = [c_void_p, c_char_p, c_int, c_int,
                                        c_int]

yogi.YOGI_ConfigurationWriteToFile.restype = api_result_handler
yogi.YOGI_ConfigurationWriteToFile.argtypes = [c_void_p, c_char_p, c_int,
                                               c_int]


def run_with_discriptive_failure_awareness(fn):
    err = create_string_buffer(256)
    try:
        fn(err)
    except Failure as failure:
        description = err.value.decode("utf-8")
        if len(description):
            raise DescriptiveFailure(failure.value, description)
        else:
            raise


class Configuration(Object):
    """Stores program parameters from different sources.

    A configuration represents a set of parameters that usually remain constant
    throughout the runtime of a program. Parameters can come from different
    sources such as the command line or a file. Configurations are used for
    other parts of the library such as application objects, however, they are
    also intended to store user-defined parameters.
    """

    def __init__(self, flags: ConfigurationFlags = ConfigurationFlags.NONE):
        """Create a configuration.

        Args:
            flags: Flags for changing the configuration's behaviour.
        """
        handle = c_void_p()
        yogi.YOGI_ConfigurationCreate(byref(handle), flags)
        Object.__init__(self, handle)
        self._flags = flags

    @property
    def flags(self) -> ConfigurationFlags:
        """Configuration flags."""
        return self._flags

    def update_from_command_line(self, argv: List[str],
                                 options: CommandLineOptions
                                 = ConfigurationFlags.NONE) -> None:
        """Updates the configuration from command line options.

        If parsing the command line, files or any given JSON string fails, or
        if help is requested (e.g. by using the --help switch) then a
        DescriptiveFailure exception will be raised containing detailed
        information about the error or the help text.

        Args:
            argv:    List of command line arguments including the script name.
            options: Options to provide on the command line.
        """
        args = (POINTER(c_char) * (len(argv) + 1))()
        for i, arg in enumerate(argv):
            args[i] = create_string_buffer(arg.encode("utf-8"))

        run_with_discriptive_failure_awareness(
            lambda err: yogi.YOGI_ConfigurationUpdateFromCommandLine(
                self._handle, len(args) - 1, args, options, err, sizeof(err)))

    def update_from_json(self, jsn: Union[str, object]) -> None:
        """Updates the configuration from a JSON data.

        If parsing fails then a DescriptiveFailure exception will be raised
        containing detailed information about the error.

        Args:
            jsn: Serializable object or JSON-formatted string.
        """
        if not isinstance(jsn, str):
            jsn = json.dumps(jsn)

        run_with_discriptive_failure_awareness(
            lambda err: yogi.YOGI_ConfigurationUpdateFromJson(
                self._handle, jsn.encode("utf-8"), err, sizeof(err)))

    def update_from_file(self, filename: str) -> None:
        """Updates the configuration from a JSON file.

        If parsing the file fails then a DescriptiveFailure exception will be
        raised containing detailed information about the error.

        Args:
            filename: Path to the JSON file.
        """
        run_with_discriptive_failure_awareness(
            lambda err: yogi.YOGI_ConfigurationUpdateFromFile(
                self._handle, filename.encode("utf-8"), err, sizeof(err)))

    def dump(self, resolve_variables: Optional[bool] = None,
             indentation: Optional[int] = None) -> str:
        """Retrieves the configuration as a JSON-formatted string.

        Args:
            resolve_variables: Resolve all configuration variables.
            indentation:       Number of space characters to use for
                               indentation. A value of None uses no spaces
                               and omits new lines as well.

        Returns:
            The configuration as a JSON-formatted string.
        """
        if resolve_variables is None:
            if self._flags & ConfigurationFlags.DISABLE_VARIABLES:
                resolve_variables = False
            else:
                resolve_variables = True

        if indentation is None:
            indentation = -1

        s = create_string_buffer(256)
        while True:
            try:
                yogi.YOGI_ConfigurationDump(self._handle, s, sizeof(s),
                                            resolve_variables, indentation)
                break
            except Failure as failure:
                if failure.error_code is ErrorCode.BUFFER_TOO_SMALL:
                    s = create_string_buffer(sizeof(s) * 2)
                else:
                    raise

        return s.value.decode("utf-8")

    def write_to_file(self, filename: str,
                      resolve_variables: Optional[bool] = None,
                      indentation: Optional[int] = None) -> str:
        """Writes the configuration to a file in JSON format.

        This is useful for debugging purposes.

        Args:
            filename:          Path to the output file.
            resolve_variables: Resolve all configuration variables.
            indentation:       Number of space characters to use for
                               indentation. A value of None uses no spaces
                               and omits new lines as well.

        Returns:
            The configuration as a JSON-formatted string.
        """
        if resolve_variables is None:
            if self._flags & ConfigurationFlags.DISABLE_VARIABLES:
                resolve_variables = False
            else:
                resolve_variables = True

        if indentation is None:
            indentation = -1

        yogi.YOGI_ConfigurationWriteToFile(
            self._handle, filename.encode("utf-8"), resolve_variables,
            indentation)
