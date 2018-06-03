#include "connection_manager.h"
#include "../../../utils/crypto.h"

#include <boost/uuid/uuid_io.hpp>

namespace objects {
namespace detail {

ConnectionManager::ConnectionManager(
    ContextPtr context, const std::string& password,
    const boost::asio::ip::udp::endpoint& adv_ep,
    ConnectionChangedHandler connection_changed_handler)
    : context_(context),
      password_hash_(utils::MakeSharedByteVector(
          utils::MakeSha256({password.cbegin(), password.cend()}))),
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
    if (entry.second) {
      branches.push_back(
          std::make_pair(entry.first, entry.second->MakeInfoString()));
    }
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

  StartExchangeBranchInfo(socket, {});
  StartAccept();
}

void ConnectionManager::OnAdvertisementReceived(
    const boost::uuids::uuid& adv_uuid,
    const boost::asio::ip::tcp::endpoint& ep) {
  std::lock_guard<std::mutex> lock(connections_mutex_);
  if (blacklisted_uuids_.count(adv_uuid)) return;

  auto res =
      connections_.insert(std::make_pair(adv_uuid, BranchConnectionPtr{}));
  if (!res.second) return;
  auto entry = &*res.first;

  auto socket = MakeSocket();
  socket->Connect(ep, [this, entry, ep, socket](auto& err) {
    this->OnConnectFinished(err, entry, ep, socket);
  });

  EmitBranchEvent(kBranchDiscoveredEvent, api::kSuccess, adv_uuid, [&] {
    return nlohmann::json{{"uuid", boost::uuids::to_string(adv_uuid)},
                          {"tcp_server_address", ep.address().to_string()},
                          {"tcp_server_port", ep.port()}};
  });
}

void ConnectionManager::OnConnectFinished(
    const api::Error& err, ConnectionsMapEntry* entry,
    const boost::asio::ip::tcp::endpoint& ep, utils::TimedTcpSocketPtr socket) {
  YOGI_ASSERT(entry != nullptr && entry->first != boost::uuids::uuid{});

  std::lock_guard<std::mutex> lock(connections_mutex_);

  if (err && !entry->second) {
    auto& adv_uuid = entry->first;
    EmitBranchEvent(kBranchQueriedEvent, err, adv_uuid, [&] {
      return nlohmann::json{{"uuid", boost::uuids::to_string(adv_uuid)}};
    });

    connections_.erase(adv_uuid);  // pointer "entry" is invalid after this

    return;
  }

  StartExchangeBranchInfo(socket, entry);
}

void ConnectionManager::StartExchangeBranchInfo(utils::TimedTcpSocketPtr socket,
                                                ConnectionsMapEntry* entry) {
  auto conn = std::make_shared<BranchConnection>(socket, info_);
  conn->ExchangeBranchInfo([this, conn, entry](auto& err) {
    this->OnExchangeBranchInfoFinished(err, conn, entry);
  });
}

void ConnectionManager::OnExchangeBranchInfoFinished(
    const api::Error& err, BranchConnectionPtr conn,
    ConnectionsMapEntry* entry) {
  std::lock_guard<std::mutex> lock(connections_mutex_);

  if (err) {
    PublishExchangeBranchInfoError(err, conn, entry);
    EraseConnectionIfNotAlreadyEstablished(entry);
    return;
  }

  // UUIDs from advertising message and info message don't match
  CheckAndFixUuidMismatch(conn->GetRemoteBranchInfo()->GetUuid(), &entry);
  YOGI_ASSERT(entry != nullptr);

  if (DoesHigherPriorityConnectionExist(*entry)) {
    return;
  }

  entry->second = conn;

  EmitBranchEvent(kBranchQueriedEvent, api::kSuccess, entry->first,
                  [&] { return conn->GetRemoteBranchInfo()->ToJson(); });

  // TODO: check name, path, net_name, etc...
  StartAuthenticate(conn);
}

void ConnectionManager::PublishExchangeBranchInfoError(
    const api::Error& err, BranchConnectionPtr conn,
    ConnectionsMapEntry* entry) {
  if (entry) {  // Connect operation failed
    EmitBranchEvent(kBranchQueriedEvent, err, entry->first, [&] {
      return nlohmann::json{{"uuid", boost::uuids::to_string(entry->first)}};
    });
  } else {
    YOGI_LOG_WARNING(logger_,
                     info_ << " Exchanging branch info with "
                           << conn->GetRemoteEndpoint().address().to_string()
                           << " failed: " << err);
  }
}

void ConnectionManager::EraseConnectionIfNotAlreadyEstablished(
    ConnectionsMapEntry* entry) {
  bool origin_is_tcp_server = entry == nullptr;
  if (origin_is_tcp_server) return;

  bool conn_already_exists = !!entry->second;
  if (conn_already_exists) return;

  connections_.erase(entry->first);
}

void ConnectionManager::CheckAndFixUuidMismatch(const boost::uuids::uuid& uuid,
                                                ConnectionsMapEntry** entry) {
  if (*entry) {
    if ((*entry)->first == uuid) return;
    YOGI_LOG_INFO(logger_, info_ << "Tried to connect to {" << (*entry)->first
                                 << "} but ended up connecting to {" << uuid
                                 << "} instead");
  }

  EraseConnectionIfNotAlreadyEstablished(*entry);

  auto res = connections_.insert(std::make_pair(uuid, BranchConnectionPtr{}));
  *entry = &*res.first;
}

bool ConnectionManager::DoesHigherPriorityConnectionExist(
    const ConnectionsMapEntry& entry) const {
  return entry.second && entry.first < info_->GetUuid();
}

void ConnectionManager::StartAuthenticate(BranchConnectionPtr connection) {
  connection->Authenticate(password_hash_, [this, connection](auto& err) {
    this->OnAuthenticateFinished(err, connection);
  });
}

void ConnectionManager::OnAuthenticateFinished(const api::Error& err,
                                               BranchConnectionPtr connection) {
  YOGI_LOG_FATAL(logger_, info_ << " Authentication: " << err);
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
                           << ev_res << "; json=\"" << make_json_fn() << "\"");
      break;

    case kBranchQueriedEvent:
      YOGI_LOG_INFO(logger_,
                    info_ << " Event: YOGI_BEV_BRANCH_QUERIED; ev_res=\""
                          << ev_res << "; json=\"" << make_json_fn() << "\"");
      break;

    case kConnectFinishedEvent:
      YOGI_LOG_INFO(logger_,
                    info_ << " Event: YOGI_BEV_CONNECT_FINISHED; ev_res=\""
                          << ev_res << "; json=\"" << make_json_fn() << "\"");
      break;

    case kConnectionLostEvent:
      YOGI_LOG_WARNING(
          logger_, info_ << " Event: YOGI_BEV_CONNECTION_LOST; ev_res=\""
                         << ev_res << "; json=\"" << make_json_fn() << "\"");
      break;
  }
}

const LoggerPtr ConnectionManager::logger_ =
    Logger::CreateStaticInternalLogger("Branch.ConnectionManager");

}  // namespace detail
}  // namespace objects
