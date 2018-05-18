#pragma once

#include "../../../config.h"
#include "tcp_base.h"

namespace objects {
namespace detail {

class TcpClient : public TcpBase {
 public:
  using TcpBase::TcpBase;

  void Connect(const boost::asio::ip::tcp::endpoint& ep);

 private:
  void OnConnected(utils::TimedTcpSocketPtr socket);
};

typedef std::shared_ptr<TcpClient> TcpClientPtr;

}  // namespace detail
}  // namespace objects
