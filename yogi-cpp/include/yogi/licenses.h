#pragma once

#include "internal/library.h"


namespace yogi {

YOGI_DEFINE_API_FN(const char*, YOGI_GetLicense, ())
YOGI_DEFINE_API_FN(const char*, YOGI_Get3rdPartyLicenses, ())

inline const std::string& GetLicense() {
  static std::string s = internal::YOGI_GetLicense();
  return s;
}

inline const std::string& Get3rdPartyLicenses() {
  static std::string s = internal::YOGI_Get3rdPartyLicenses();
  return s;
}

}  // namespace yogi
