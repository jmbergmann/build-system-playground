#include "tcp_server.h"

namespace objects {
namespace detail {

TcpServer::TcpServer(ContextPtr context, LocalBranchInfoPtr info,
                     SuccessHandler&& success_handler, ErrorHandler&& error_handler)
    : TcpBase(context, info, std::move(success_handler), std::move(error_handler)),
      acceptor_(context->IoContext()) {
  SetupAcceptor();
}

void TcpServer::Start() {
  TcpBase::Start();
  StartAccept();
}

void TcpServer::SetupAcceptor() {
  boost::system::error_code ec;

  auto protocol = GetInfo()->adv_ep.protocol() == boost::asio::ip::udp::v4()
                      ? boost::asio::ip::tcp::v4()
                      : boost::asio::ip::tcp::v6();
  auto ep = boost::asio::ip::tcp::endpoint(protocol, 0);

  acceptor_.open(ep.protocol(), ec);
  if (ec) {
    throw api::Error(YOGI_ERR_OPEN_SOCKET_FAILED);
  }

  acceptor_.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true), ec);
  if (ec) {
    throw api::Error(YOGI_ERR_SET_SOCKET_OPTION_FAILED);
  }

  acceptor_.bind(ep, ec);
  if (ec) {
    throw api::Error(YOGI_ERR_BIND_SOCKET_FAILED);
  }

  GetInfo()->tcp_ep = acceptor_.local_endpoint();

  acceptor_.listen(acceptor_.max_listen_connections, ec);
  if (ec) {
    throw api::Error(YOGI_ERR_LISTEN_SOCKET_FAILED);
  }
}

void TcpServer::StartAccept() {
  auto socket =
      std::make_shared<utils::TimedTcpSocket>(GetContext(), GetInfo()->timeout);
  auto weak_self = MakeWeakPtr<TcpServer>();
  socket->Accept(&acceptor_, [weak_self, socket](const auto& err) {
    auto self = weak_self.lock();
    if (!self) return;

    if (!err) {
      self->OnAccepted(socket);
    } else {
      self->CallErrorHandler(err, socket);
    }
  });
}

void TcpServer::OnAccepted(utils::TimedTcpSocketPtr socket) {
  YOGI_LOG_DEBUG(GetLogger(),
                 "Incoming connection from " << *socket << " accepted");
  StartAccept();
  StartInfoExchange(socket);
}

}  // namespace detail
}  // namespace objects
