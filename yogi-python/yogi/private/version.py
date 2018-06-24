from .library import yogi

from ctypes import c_char_p

yogi.YOGI_GetVersion.restype = c_char_p
yogi.YOGI_GetVersion.argtypes = []


def get_version() -> str:
    """Retrieve the Yogi Core version.

    Returns:
        Version string of the loaded Yogi Core library.
    """
    return yogi.YOGI_GetVersion().decode()
