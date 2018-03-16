#include "constants.h"
#include "errors.h"
#include "../include/yogi_core.h"

namespace constants {

void GetConstant(void* dest, int constant) {
  switch (constant) {
    case YOGI_CONST_VERSION_NUMBER:
      *static_cast<const char**>(dest) = constants::kVersionNumber;
      break;

    case YOGI_CONST_VERSION_MAJOR:
      *static_cast<int*>(dest) = constants::kVersionMajor;
      break;

    case YOGI_CONST_VERSION_MINOR:
      *static_cast<int*>(dest) = constants::kVersionMinor;
      break;

    case YOGI_CONST_VERSION_PATCH:
      *static_cast<int*>(dest) = constants::kVersionPatch;
      break;

    case YOGI_CONST_DEFAULT_ADV_PORT:
      *static_cast<int*>(dest) = constants::kDefaultAdvPort;
      break;

    case YOGI_CONST_DEFAULT_ADV_INTERVAL:
      *static_cast<int*>(dest) = constants::kDefaultAdvInterval;
      break;

    default:
      throw errors::Error(YOGI_ERR_INVALID_PARAM);
  }
}

} // namespace constants
