#pragma once

#include "internal/library.h"


namespace yogi {

YOGI_DEFINE_API_FN(const char*, YOGI_GetLicense, ())
YOGI_DEFINE_API_FN(const char*, YOGI_Get3rdPartyLicenses, ())

//! Get the license that Yogi is published under.
//!
//! Note: The returned string contains the complete description of the license
//!       and is therefore very large.
//!
//! \returns License information
inline const std::string& GetLicense() {
  static std::string s = internal::YOGI_GetLicense();
  return s;
}

//! Get the license information about the 3rd party libraries used in Yogi.
//!
//! Note: The returned string is very large.
//!
//! \returns License information
inline const std::string& Get3rdPartyLicenses() {
  static std::string s = internal::YOGI_Get3rdPartyLicenses();
  return s;
}

}  // namespace yogi
