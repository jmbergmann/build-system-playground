#include "tcp_client.h"

namespace objects {
namespace detail {

const LoggerPtr TcpClient::logger_ =
    Logger::CreateStaticInternalLogger("Branch");

TcpClient::TcpClient(ContextPtr context, LocalBranchInfoPtr info,
                     ObserverFn&& observer_fn)
    : context_(context), info_(info), observer_fn_(observer_fn) {}

void TcpClient::Connect(const boost::asio::ip::tcp::endpoint& ep,
                        std::function<void(const api::Error&)>&& handler) {
  auto peer = std::make_shared<RemoteBranchInfo>();
  peer->tcp_ep = ep;
  peer->socket =
      std::make_shared<utils::TimedTcpSocket>(context_, info_->timeout);

  auto weak_self = std::weak_ptr<TcpClient>{shared_from_this()};
  peer->socket->Connect(ep, [weak_self, peer](const auto& err) {
    auto self = weak_self.lock();
    if (!self) return;

    if (!err) {
      self->OnConnected(peer);
    } else {
      YOGI_LOG_ERROR(self->logger_, "Connecting to "
                                        << peer->tcp_ep.address().to_string()
                                        << ':' << peer->tcp_ep.port()
                                        << " failed: " << err);
    }
  });
}

void TcpClient::OnConnected(RemoteBranchInfoPtr peer) { YOGI_TRACE; }

}  // namespace detail
}  // namespace objects
