#include "ip.h"

namespace utils {

std::string MakeIpAddressString(const boost::asio::ip::address& addr) {
  auto s = addr.to_string();

  auto pos = s.find('%');
  if (pos != std::string::npos) {
    s.erase(pos);
  }

  return s;
}

std::string MakeIpAddressString(const boost::asio::ip::tcp::endpoint& ep) {
  return MakeIpAddressString(ep.address());
}

std::string MakeIpAddressString(const boost::asio::ip::udp::endpoint& ep) {
  return MakeIpAddressString(ep.address());
}

}  // namespace utils
