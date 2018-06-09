#pragma once

#include "../config.h"

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ip/udp.hpp>

namespace utils {

std::string MakeIpAddressString(const boost::asio::ip::address& addr);
std::string MakeIpAddressString(const boost::asio::ip::tcp::endpoint& ep);
std::string MakeIpAddressString(const boost::asio::ip::udp::endpoint& ep);

}  // namespace utils
