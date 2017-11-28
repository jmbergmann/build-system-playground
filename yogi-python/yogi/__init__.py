import sys as __sys

if __sys.version_info.major == 3:
    assert __sys.version_info.minor >= 5, "YOGI for Python requires Python version 3.5 or higher." \
        " This is version {}.{}.".format(__sys.version_info.major, __sys.version_info.minor)

from .private.version import get_version
