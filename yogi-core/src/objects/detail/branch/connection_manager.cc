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
          [&](auto& uuid, auto& ep) { OnAdvertisementReceived(uuid, ep); })),
      observed_events_(kNoEvent),
      cancel_await_event_running_(false) {
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
  std::lock_guard<std::recursive_mutex> lock(event_mutex_);

  if (cancel_await_event_running_) {
    throw api::Error(YOGI_ERR_BUSY);
  }

  if (event_handler_) {
    CancelAwaitEvent();
  }

  observed_events_ = events;
  event_handler_ = handler;
}

void ConnectionManager::CancelAwaitEvent() {
  std::lock_guard<std::recursive_mutex> lock(event_mutex_);

  if (cancel_await_event_running_) {
    throw api::Error(YOGI_ERR_BUSY);
  }

  cancel_await_event_running_ = true;

  event_handler_(api::Error(YOGI_ERR_CANCELED), kNoEvent, api::kSuccess, {},
                 {});

  event_handler_ = {};
  cancel_await_event_running_ = false;
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

  StartExchangeBranchInfo(socket, {}, true);
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

  auto socket = MakeSocket();
  socket->Connect(ep, [this, uuid, ep, socket](auto& err) {
    this->OnConnectFinished(err, uuid, ep, socket);
  });

  EmitBranchEvent(kBranchDiscoveredEvent, api::kSuccess, uuid, [&] {
    return nlohmann::json{{"uuid", boost::uuids::to_string(uuid)},
                          {"tcp_server_address", ep.address().to_string()},
                          {"tcp_server_port", ep.port()}};
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

    EmitBranchEvent(kBranchQueriedEvent, err, uuid, [&] {
      return nlohmann::json{{"uuid", boost::uuids::to_string(uuid)}}.dump();
    });

    return;
  }

  StartExchangeBranchInfo(socket, uuid, false);
}

void ConnectionManager::StartExchangeBranchInfo(utils::TimedTcpSocketPtr socket,
                                                const boost::uuids::uuid& uuid,
                                                bool origin_is_tcp_server) {
  auto connection = std::make_shared<BranchConnection>(socket, info_);
  connection->ExchangeBranchInfo(
      [this, connection, origin_is_tcp_server, uuid](auto& err) {
        this->OnExchangeBranchInfoFinished(err, connection, uuid,
                                           origin_is_tcp_server);
      });
}

void ConnectionManager::OnExchangeBranchInfoFinished(
    const api::Error& err, BranchConnectionPtr connection,
    const boost::uuids::uuid& uuid, bool origin_is_tcp_server) {
  // TODO: authenticate

  if (!origin_is_tcp_server) return;

  if (connection->GetRemoteBranchInfo()) {
    EmitBranchEvent(
        kBranchQueriedEvent, err, connection->GetRemoteBranchInfo()->GetUuid(),
        [&] { return connection->GetRemoteBranchInfo()->ToJson().dump(); });
  } else {
    EmitBranchEvent(kBranchQueriedEvent, err, uuid, [&] {
      return nlohmann::json{{"uuid", boost::uuids::to_string(uuid)}}.dump();
    });
  }
}

utils::TimedTcpSocketPtr ConnectionManager::MakeSocket() {
  return std::make_shared<utils::TimedTcpSocket>(context_, info_->GetTimeout());
}

template <typename Fn>
void ConnectionManager::EmitBranchEvent(BranchEvents event,
                                        const api::Error& ev_res,
                                        const boost::uuids::uuid& uuid,
                                        Fn make_json_fn) {
  LogBranchEvent(event, ev_res, uuid, make_json_fn);

  std::lock_guard<std::recursive_mutex> lock(event_mutex_);

  if (!event_handler_) return;
  if (!(observed_events_ & event)) return;

  auto handler = event_handler_;
  event_handler_ = {};
  handler(api::kSuccess, event, ev_res, uuid, make_json_fn());
}

template <typename Fn>
void ConnectionManager::LogBranchEvent(BranchEvents event,
                                       const api::Error& ev_res,
                                       const boost::uuids::uuid& uuid,
                                       Fn make_json_fn) {
  switch (event) {
    case kBranchDiscoveredEvent:
      YOGI_LOG_DEBUG(logger_,
                     info_ << " Event: YOGI_BEV_BRANCH_DISCOVERED; ev_res=\""
                           << ev_res << "\"; uuid=" << uuid << "; json=\""
                           << make_json_fn() << "\"");
      break;

    case kBranchQueriedEvent:
      YOGI_LOG_INFO(logger_, info_
                                 << " Event: YOGI_BEV_BRANCH_QUERIED; ev_res=\""
                                 << ev_res << "\"; uuid=" << uuid << "; json=\""
                                 << make_json_fn() << "\"");
      break;

    case kConnectFinishedEvent:
      YOGI_LOG_INFO(logger_,
                    info_ << " Event: YOGI_BEV_CONNECT_FINISHED; ev_res=\""
                          << ev_res << "\"; uuid=" << uuid << "; json=\""
                          << make_json_fn() << "\"");
      break;

    case kConnectionLostEvent:
      YOGI_LOG_WARNING(logger_,
                       info_ << " Event: YOGI_BEV_CONNECTION_LOST; ev_res=\""
                             << ev_res << "\"; uuid=" << uuid << "; json=\""
                             << make_json_fn() << "\"");
      break;
  }
}

const LoggerPtr ConnectionManager::logger_ =
    Logger::CreateStaticInternalLogger("Branch.ConnectionManager");

}  // namespace detail
}  // namespace objects
