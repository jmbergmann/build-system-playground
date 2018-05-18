#include "tcp_client.h"

namespace objects {
namespace detail {

void TcpClient::Connect(const boost::asio::ip::tcp::endpoint& ep) {
  auto socket =
      std::make_shared<utils::TimedTcpSocket>(GetContext(), GetInfo()->timeout);

  auto weak_self = MakeWeakPtr<TcpClient>();
  socket->Connect(ep, [weak_self, socket](const auto& err) {
    auto self = weak_self.lock();
    if (!self) return;

    if (!err) {
      self->OnConnected(socket);
    } else {
      self->CallErrorHandler(err, socket);
    }
  });
}

void TcpClient::OnConnected(utils::TimedTcpSocketPtr socket) {
  StartInfoExchange(socket);
}

}  // namespace detail
}  // namespace objects
