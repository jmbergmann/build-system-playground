from .library import yogi

from ctypes import c_char_p

yogi.YOGI_GetLicense.restype = c_char_p
yogi.YOGI_GetLicense.argtypes = []


def get_license() -> str:
    """Retrieve the Yogi license.

    Returns:
        String containing the Yogi license.
    """
    return yogi.YOGI_GetLicense().decode()


yogi.YOGI_Get3rdPartyLicenses.restype = c_char_p
yogi.YOGI_Get3rdPartyLicenses.argtypes = []


def get_3rd_party_licenses() -> str:
    """Retrieve the licenses of the 3rd party libraries compiled into Yogi Core.

    Returns:
        String containing the 3rd party licenses.
    """
    return yogi.YOGI_Get3rdPartyLicenses().decode()
