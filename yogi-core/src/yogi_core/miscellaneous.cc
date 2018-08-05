#include "macros.h"
#include "helpers.h"
#include "../api/constants.h"
#include "../licenses/licenses.h"

YOGI_API const char* YOGI_GetVersion() { return api::kVersionNumber; }

YOGI_API const char* YOGI_GetLicense() {
  return licenses::kYogiLicense.c_str();
}

YOGI_API const char* YOGI_Get3rdPartyLicenses() {
  return licenses::k3rdPartyLicenses.c_str();
}

YOGI_API const char* YOGI_GetErrorString(int err) {
  return api::Error(err).what();
}

YOGI_API int YOGI_GetConstant(void* dest, int constant) {
  CHECK_PARAM(dest != nullptr);

  try {
    api::GetConstant(dest, constant);
  }
  CATCH_AND_RETURN;
}
