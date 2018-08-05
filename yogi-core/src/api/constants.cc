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
      // NOLINTNEXTLINE(google-runtime-int)
      *static_cast<long long*>(dest) = kDefaultAdvInterval;
      break;

    case YOGI_CONST_DEFAULT_CONNECTION_TIMEOUT:
      // NOLINTNEXTLINE(google-runtime-int)
      *static_cast<long long*>(dest) = kDefaultConnectionTimeout;
      break;

    case YOGI_CONST_DEFAULT_LOGGER_VERBOSITY:
      *static_cast<int*>(dest) = kDefaultLoggerVerbosity;
      break;

    case YOGI_CONST_DEFAULT_LOG_TIME_FORMAT:
      *static_cast<const char**>(dest) = kDefaultLogTimeFormat;
      break;

    case YOGI_CONST_DEFAULT_LOG_FORMAT:
      *static_cast<const char**>(dest) = kDefaultLogFormat;
      break;

    case YOGI_CONST_MAX_MESSAGE_SIZE:
      *static_cast<int*>(dest) = kMaxMessageSize;
      break;

    case YOGI_CONST_DEFAULT_TIME_FORMAT:
      *static_cast<const char**>(dest) = kDefaultTimeFormat;
      break;

    case YOGI_CONST_DEFAULT_INF_DURATION_STRING:
      *static_cast<const char**>(dest) = kDefaultInfiniteDurationString;
      break;

    case YOGI_CONST_DEFAULT_DURATION_FORMAT:
      *static_cast<const char**>(dest) = kDefaultDurationFormat;
      break;

    case YOGI_CONST_DEFAULT_INVALID_HANDLE_STRING:
      *static_cast<const char**>(dest) = kDefaultInvalidHandleString;
      break;

    case YOGI_CONST_DEFAULT_OBJECT_FORMAT:
      *static_cast<const char**>(dest) = kDefaultObjectFormat;
      break;

    default:
      throw Error(YOGI_ERR_INVALID_PARAM);
  }
}

} // namespace api
