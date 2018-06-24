from enum import IntEnum


class ConfigurationOptions(IntEnum):
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
                           .line
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
