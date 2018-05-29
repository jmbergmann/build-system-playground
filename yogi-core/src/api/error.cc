#include "error.h"

namespace api {

const char* Error::what() const noexcept {
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

    case YOGI_ERR_INVALID_HANDLE:
      return "Invalid handle";

    case YOGI_ERR_WRONG_OBJECT_TYPE:
      return "Wrong object type";

    case YOGI_ERR_CANCELED:
      return "Operation has been canceled";

    case YOGI_ERR_BUSY:
      return "Object is busy";

    case YOGI_ERR_TIMEOUT:
      return "The operation timed out";

    case YOGI_ERR_TIMER_EXPIRED:
      return "The timer has not been started or already expired";

    case YOGI_ERR_BUFFER_TOO_SMALL:
      return "The supplied buffer is too small";

    case YOGI_ERR_OPEN_SOCKET_FAILED:
      return "Could not open a socket";

    case YOGI_ERR_BIND_SOCKET_FAILED:
      return "Could not bind a socket";

    case YOGI_ERR_LISTEN_SOCKET_FAILED:
      return "Could not listen on socket";

    case YOGI_ERR_SET_SOCKET_OPTION_FAILED:
      return "Could not set a socket option";

    case YOGI_ERR_INVALID_REGEX:
      return "Invalid regular expression";

    case YOGI_ERR_OPEN_FILE_FAILED:
      return "Could not open file";

    case YOGI_ERR_RW_SOCKET_FAILED:
      return "Could not read from or write to socket";

    case YOGI_ERR_CONNECT_SOCKET_FAILED:
      return "Could not connect a socket";

    case YOGI_ERR_INVALID_MAGIC_PREFIX:
      return "The magic prefix sent when establishing a connection is wrong";

    case YOGI_ERR_INCOMPATIBLE_VERSION:
      return "The local and remote branches use incompatible Yogi versions";

    case YOGI_ERR_DESERIALIZE_MSG_FAILED:
      return "Could not deserialize a message";

    case YOGI_ERR_ACCEPT_SOCKET_FAILED:
      return "Could not accept a socket";

    case YOGI_ERR_LOOPBACK_CONNECTION:
      return "Attempting to connect branch to itself";
  }

  return "Invalid error code";
}

} // namespace api

std::ostream& operator<< (std::ostream& os, const api::Error& err) {
  os << err.what();
  return os;
}
