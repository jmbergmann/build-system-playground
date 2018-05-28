#include "connection_manager.h"

#include <boost/uuid/uuid_io.hpp>

namespace objects {
namespace detail {

ConnectionManager::ConnectionManager(
    ContextPtr context, const boost::asio::ip::udp::endpoint& adv_ep,
    ConnectionChangedHandler connection_changed_handler)
    : context_(context),
      connection_changed_handler_(connection_changed_handler),
      acceptor_(context->IoContext()),
      adv_sender_(std::make_shared<AdvertisingSender>(context, adv_ep)),
      adv_receiver_(std::make_shared<AdvertisingReceiver>(
          context, adv_ep,
          [&](auto& uuid, auto& ep) { OnAdvertisementReceived(uuid, ep); })) {
  using namespace boost::asio::ip;
  SetupAcceptor(adv_ep.protocol() == udp::v4() ? tcp::v4() : tcp::v6());
}

void ConnectionManager::Start(BranchInfoPtr info) {
  info_ = info;
  StartAccept();
  adv_sender_->Start(info);
  adv_receiver_->Start(info);
}

ConnectionManager::BranchInfoStringsList
ConnectionManager::MakeConnectedBranchesInfoStrings() const {
  BranchInfoStringsList branches;

  std::lock_guard<std::mutex> lock(connections_mutex_);
  for (auto& entry : connections_) {
    branches.push_back(
        std::make_pair(entry.first, entry.second->MakeInfoString()));
  }

  return branches;
}

void ConnectionManager::AwaitEvent(BranchEvents events,
                                   BranchEventHandler handler) {
  std::lock_guard<std::recursive_mutex> lock(event_handler_mutex_);
  CancelAwaitEvent();
  event_handler_ = handler;
}

void ConnectionManager::CancelAwaitEvent() {
  std::lock_guard<std::recursive_mutex> lock(event_handler_mutex_);
  if (event_handler_) {
    event_handler_(api::Error(YOGI_ERR_CANCELED), kNoEvent, api::kSuccess, {},
                   {});
    event_handler_ = {};
  }
}

void ConnectionManager::SetupAcceptor(const boost::asio::ip::tcp& protocol) {
  boost::system::error_code ec;
  acceptor_.open(protocol, ec);
  if (ec) throw api::Error(YOGI_ERR_OPEN_SOCKET_FAILED);

  acceptor_.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true), ec);
  if (ec) throw api::Error(YOGI_ERR_SET_SOCKET_OPTION_FAILED);

  acceptor_.bind(boost::asio::ip::tcp::endpoint(protocol, 0), ec);
  if (ec) throw api::Error(YOGI_ERR_BIND_SOCKET_FAILED);

  acceptor_.listen(acceptor_.max_listen_connections, ec);
  if (ec) throw api::Error(YOGI_ERR_LISTEN_SOCKET_FAILED);
}

void ConnectionManager::StartAccept() {
  auto socket = MakeSocket();

  auto weak_self = std::weak_ptr<ConnectionManager>{shared_from_this()};
  socket->Accept(&acceptor_, [weak_self, socket](auto& err) {
    auto self = weak_self.lock();
    if (!self) return;

    self->OnAcceptFinished(err, socket);
  });
}

void ConnectionManager::OnAcceptFinished(const api::Error& err,
                                         utils::TimedTcpSocketPtr socket) {
  if (err) {
    YOGI_LOG_ERROR(logger_,
                   info_ << " Accepting incoming TCP connection failed: " << err
                         << ". No more connections will be accepted.");
    return;
  }

  StartExchangeBranchInfo(socket, true);
  StartAccept();
}

void ConnectionManager::OnAdvertisementReceived(
    const boost::uuids::uuid& uuid, const boost::asio::ip::tcp::endpoint& ep) {
  {
    std::lock_guard<std::mutex> lock(connections_mutex_);
    if (blacklisted_uuids_.count(uuid) || connections_.count(uuid)) {
      return;
    }

    connections_[uuid] = {};
  }

  YOGI_LOG_INFO(logger_, info_ << " New branch " << uuid << " discovered on "
                               << ep.address().to_string() << ':' << ep.port())

  auto socket = MakeSocket();
  socket->Connect(ep, [this, uuid, ep, socket](auto& err) {
    this->OnConnectFinished(err, uuid, ep, socket);
  });
}

void ConnectionManager::OnConnectFinished(
    const api::Error& err, const boost::uuids::uuid& uuid,
    const boost::asio::ip::tcp::endpoint& ep, utils::TimedTcpSocketPtr socket) {
  if (err) {
    YOGI_LOG_ERROR(logger_, info_ << " Could not connect to branch " << uuid
                                  << " on " << ep.address().to_string() << ':'
                                  << ep.port() << ": " << err);

    std::lock_guard<std::mutex> lock(connections_mutex_);
    auto it = connections_.find(uuid);
    bool accepted_by_tcp_server = it != connections_.end() && it->second;
    if (!accepted_by_tcp_server) {
      connections_.erase(it);
    }

    return;
  }

  StartExchangeBranchInfo(socket, false);
}

void ConnectionManager::StartExchangeBranchInfo(utils::TimedTcpSocketPtr socket,
                                                bool origin_is_tcp_server) {
  auto connection = std::make_shared<BranchConnection>(socket, info_);
  connection->ExchangeBranchInfo([this, connection,
                                  origin_is_tcp_server](auto& err) {
    this->OnExchangeBranchInfoFinished(err, connection, origin_is_tcp_server);
  });
}

void ConnectionManager::OnExchangeBranchInfoFinished(
    const api::Error& err, BranchConnectionPtr connection,
    bool origin_is_tcp_server) {
  // TODO
}

utils::TimedTcpSocketPtr ConnectionManager::MakeSocket() {
  return std::make_shared<utils::TimedTcpSocket>(context_, info_->GetTimeout());
}

const LoggerPtr ConnectionManager::logger_ =
    Logger::CreateStaticInternalLogger("Branch.ConnectionManager");

}  // namespace detail
}  // namespace objects
