#pragma once

#include "../../../config.h"
#include "tcp_base.h"

namespace objects {
namespace detail {

class TcpClient : public TcpBase {
 public:
  using TcpBase::TcpBase;

  void Connect(const boost::asio::ip::tcp::endpoint& ep,
               std::function<void(const api::Error&)>&& handler);

 private:
  void OnConnected(RemoteBranchInfoPtr peer);
};

typedef std::shared_ptr<TcpClient> TcpClientPtr;

}  // namespace detail
}  // namespace objects
