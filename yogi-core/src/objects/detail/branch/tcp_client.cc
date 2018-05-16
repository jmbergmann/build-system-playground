#include "tcp_client.h"

namespace objects {
namespace detail {

void TcpClient::Connect(const boost::asio::ip::tcp::endpoint& ep,
                        std::function<void(const api::Error&)>&& handler) {
  auto socket =
      std::make_shared<utils::TimedTcpSocket>(GetContext(), GetInfo()->timeout);

  auto weak_self = MakeWeakPtr<TcpClient>();
  socket->Connect(ep, [weak_self, ep, socket](const auto& err) {
    auto self = weak_self.lock();
    if (!self) return;

    if (!err) {
      self->OnConnected(socket);
    } else {
      YOGI_LOG_ERROR(self->GetLogger(), "Connecting to "
                                            << ep.address().to_string() << ':'
                                            << ep.port() << " failed: " << err);
    }
  });
}

void TcpClient::OnConnected(utils::TimedTcpSocketPtr socket) {
  StartInfoExchange(socket);
}

}  // namespace detail
}  // namespace objects
