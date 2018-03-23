#include "error.h"

namespace api {

const char* Error::what() const throw() {
  if (err_ >= 0) {
    return "Success";
  }

  switch (err_) {
    case YOGI_ERR_UNKNOWN:
      return "Unknown internal error";

    case YOGI_ERR_OBJECT_STILL_USED:
      return "Object is still being used by another object";

    case YOGI_ERR_BAD_ALLOC:
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

} // namespace api
