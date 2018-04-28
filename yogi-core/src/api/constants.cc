#include "constants.h"
#include "error.h"

namespace api {

void GetConstant(void* dest, int constant) {
  switch (constant) {
    case YOGI_CONST_VERSION_NUMBER:
      *static_cast<const char**>(dest) = kVersionNumber;
      break;

    case YOGI_CONST_VERSION_MAJOR:
      *static_cast<int*>(dest) = kVersionMajor;
      break;

    case YOGI_CONST_VERSION_MINOR:
      *static_cast<int*>(dest) = kVersionMinor;
      break;

    case YOGI_CONST_VERSION_PATCH:
      *static_cast<int*>(dest) = kVersionPatch;
      break;

    case YOGI_CONST_DEFAULT_ADV_ADDRESS:
      *static_cast<const char**>(dest) = kDefaultAdvAddress;
      break;

    case YOGI_CONST_DEFAULT_ADV_PORT:
      *static_cast<int*>(dest) = kDefaultAdvPort;
      break;

    case YOGI_CONST_DEFAULT_ADV_INTERVAL:
      *static_cast<int*>(dest) = kDefaultAdvInterval;
      break;

    case YOGI_CONST_DEFAULT_TRACE_FORMAT:
      *static_cast<const char**>(dest) = kDefaultTraceFormat;
      break;

    default:
      throw Error(YOGI_ERR_INVALID_PARAM);
  }
}

} // namespace api
