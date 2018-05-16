#pragma once

#include "../../../config.h"
#include "tcp_base.h"

namespace objects {
namespace detail {

class TcpServer : public TcpBase {
 public:
  TcpServer(ContextPtr context, LocalBranchInfoPtr info, ObserverFn&& observer_fn);

  virtual void Start() override;

 private:
  void SetupAcceptor();
  void StartAccept();
  void OnAccepted(utils::TimedTcpSocketPtr socket);

  boost::asio::ip::tcp::acceptor acceptor_;
};

typedef std::shared_ptr<TcpServer> TcpServerPtr;

}  // namespace detail
}  // namespace objects
