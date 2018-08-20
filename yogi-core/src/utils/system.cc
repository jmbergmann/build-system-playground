#include "system.h"

#include <boost/asio/ip/host_name.hpp>

#ifdef _WIN32
# include <process.h>
#else
# include <sys/types.h>
# include <unistd.h>
# include <sys/syscall.h>
#endif

namespace utils {

std::string GetHostname() {
  boost::system::error_code ec;
  auto hostname = boost::asio::ip::host_name(ec);
  if (ec) {
    hostname = "unknown hostname";
  }

  return hostname;
}

int GetProcessId() {
#ifdef _WIN32
  return _getpid();
#else
  return getpid();
#endif
}

int GetCurrentThreadId() {
#ifdef _WIN32
  auto id = ::GetCurrentThreadId();
#else
  auto id = syscall(SYS_gettid);
#endif

  return static_cast<int>(id);
}

}  // namespace utils
