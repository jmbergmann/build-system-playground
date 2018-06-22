from .library import yogi
from ctypes import *

yogi.YOGI_GetLicense.restype = c_char_p
yogi.YOGI_GetLicense.argtypes = []


def get_license() -> str:
    return yogi.YOGI_GetLicense().decode()


yogi.YOGI_Get3rdPartyLicenses.restype = c_char_p
yogi.YOGI_Get3rdPartyLicenses.argtypes = []


def get_3rd_party_licenses() -> str:
    return yogi.YOGI_Get3rdPartyLicenses().decode()
