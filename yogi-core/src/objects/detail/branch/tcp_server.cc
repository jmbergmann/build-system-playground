#include "tcp_server.h"

namespace objects {
namespace detail {

const LoggerPtr TcpServer::logger_ =
    Logger::CreateStaticInternalLogger("Branch");

TcpServer::TcpServer(ContextPtr context, LocalBranchInfoPtr info)
    : context_(context), info_(info), acceptor_(context_->IoContext()) {
  SetupAcceptor();
}

void TcpServer::Start() {
  serialized_info_ = info_->MakeInfoMessage();
  StartAccept();
}

void TcpServer::SetupAcceptor() {
  boost::system::error_code ec;

  auto protocol = info_->adv_ep.protocol() == boost::asio::ip::udp::v4()
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

  info_->tcp_ep = acceptor_.local_endpoint();

  acceptor_.listen(acceptor_.max_listen_connections, ec);
  if (ec) {
    throw api::Error(YOGI_ERR_LISTEN_SOCKET_FAILED);
  }
}

void TcpServer::StartAccept() {
  auto socket =
      std::make_shared<utils::TimedTcpSocket>(context_, info_->timeout);
  auto weak_self = std::weak_ptr<TcpServer>{shared_from_this()};
  acceptor_.async_accept(socket->Socket(), [weak_self, socket](auto& ec) {
    auto self = weak_self.lock();
    if (!self) return;

    if (!ec) {
      self->OnAccepted(socket);
    } else {
      YOGI_LOG_ERROR(self->logger_,
                     "Accepting incoming connection failed: " << ec.message());
    }
  });
}

void TcpServer::OnAccepted(utils::TimedTcpSocketPtr socket) {
  YOGI_LOG_DEBUG(logger_,
                 "Incoming connection from " << *socket << " accepted");
  StartAccept();
  SendInfo(socket);
}

void TcpServer::SendInfo(utils::TimedTcpSocketPtr socket) {
  auto weak_self = std::weak_ptr<TcpServer>{shared_from_this()};
  socket->Send(boost::asio::buffer(serialized_info_), [weak_self, socket](
                                                          const auto& err) {
    auto self = weak_self.lock();
    if (!self) return;

    if (err) {
      YOGI_LOG_ERROR(self->logger_, "Could not send branch info to "
                                        << *socket << ": " << err);
    } else {
      self->OnInfoSent(socket);
    }
  });
}

void TcpServer::OnInfoSent(utils::TimedTcpSocketPtr socket) {
  YOGI_LOG_TRACE(logger_, "Local branch info sent to " << *socket);
  StartReceiveInfoHeader(socket);
}

void TcpServer::StartReceiveInfoHeader(utils::TimedTcpSocketPtr socket) {
  auto weak_self = std::weak_ptr<TcpServer>{shared_from_this()};
  socket->ReceiveExactly(
      BranchInfo::GetInfoMessageHeaderSize(),
      [weak_self, socket](const auto& err, const auto& buffer) {
        auto self = weak_self.lock();
        if (!self) return;

        if (err) {
          YOGI_LOG_ERROR(self->logger_,
                         "Could not receive branch info header from "
                             << *socket << ": " << err);
        } else {
          self->OnInfoHeaderReceived(socket, buffer);
        }
      });
}

void TcpServer::OnInfoHeaderReceived(utils::TimedTcpSocketPtr socket,
                                     const std::vector<char>& buffer) {
  YOGI_TRACE;
}

void TcpServer::StartReceiveInfoBody(utils::TimedTcpSocketPtr socket) {
  YOGI_TRACE;
}

void TcpServer::OnInfoBodyReceived(utils::TimedTcpSocketPtr socket,
                                   const std::vector<char>& buffer) {
  YOGI_TRACE;
}

}  // namespace detail
}  // namespace objects
