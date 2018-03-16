#include "errors.h"
#include "../include/yogi_core.h"

const char* GetErrorString(int err) {
  if (err >= 0) {
    return "Success";
  }

  switch (err) {
    case YOGI_ERR_UNKNOWN:
      return "Unknown internal error";

    case YOGI_ERR_OBJECT_STILL_USED:
      return "Object is still being used by another object";

    case YOGI_ERR_BAD_ALLOCATION:
      return "Memory allocation failed";

    case YOGI_ERR_INVALID_PARAM:
      return "Invalid parameter";

    case YOGI_ERR_WRONG_OBJECT_TYPE:
      return "Wrong object type";

    case YOGI_ERR_CANCELED:
      return "Operation has been canceled";
  }

  return "Invalid error code";
}
