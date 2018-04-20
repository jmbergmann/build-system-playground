#include "system.h"

#include <boost/asio/ip/host_name.hpp>

namespace utils {

std::string GetHostname() {
  boost::system::error_code ec;
  auto hostname = boost::asio::ip::host_name(ec);
  if (ec) {
    hostname = "unknown hostname";
  }

  return hostname;
}

}  // namespace utils
