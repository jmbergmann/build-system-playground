#include "tcp_client.h"

namespace objects {
namespace detail {

void TcpClient::Connect(const boost::asio::ip::tcp::endpoint& ep,
                        std::function<void(const api::Error&)>&& handler) {
  auto branch = std::make_shared<RemoteBranchInfo>();
  branch->tcp_ep = ep;
  branch->socket =
      std::make_shared<utils::TimedTcpSocket>(GetContext(), GetInfo()->timeout);

  auto weak_self = MakeWeakPtr<TcpClient>();
  branch->socket->Connect(ep, [weak_self, branch](const auto& err) {
    auto self = weak_self.lock();
    if (!self) return;

    if (!err) {
      self->OnConnected(branch);
    } else {
      YOGI_LOG_ERROR(self->GetLogger(),
                     "Connecting to " << branch->tcp_ep.address().to_string()
                                      << ':' << branch->tcp_ep.port()
                                      << " failed: " << err);
    }
  });
}

void TcpClient::OnConnected(RemoteBranchInfoPtr branch) {
  StartInfoExchange(branch);
}

}  // namespace detail
}  // namespace objects
