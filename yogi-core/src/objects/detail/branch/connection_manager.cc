#include "connection_manager.h"
#include "../../../utils/crypto.h"
#include "../../../utils/ip.h"

#include <boost/uuid/uuid_io.hpp>

namespace objects {
namespace detail {

ConnectionManager::ConnectionManager(
    ContextPtr context, const std::string& password,
    const boost::asio::ip::udp::endpoint& adv_ep,
    ConnectionChangedHandler connection_changed_handler,
    MessageHandler message_handler)
    : context_(context),
      password_hash_(utils::MakeSharedByteVector(
          utils::MakeSha256({password.cbegin(), password.cend()}))),
      connection_changed_handler_(connection_changed_handler),
      message_handler_(message_handler),
      adv_sender_(std::make_shared<AdvertisingSender>(context, adv_ep)),
      adv_receiver_(std::make_shared<AdvertisingReceiver>(
          context, adv_ep,
          [&](auto& uuid, auto& ep) {
            this->OnAdvertisementReceived(uuid, ep);
          })),
      acceptor_(context->IoContext()),
      observed_events_(kNoEvent) {
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
    if (entry.second && entry.second->SessionStarted()) {
      branches.push_back(
          std::make_pair(entry.first, entry.second->MakeInfoString()));
    }
  }

  return branches;
}

void ConnectionManager::AwaitEvent(BranchEvents events,
                                   BranchEventHandler handler) {
  std::lock_guard<std::recursive_mutex> lock(event_mutex_);

  if (event_handler_) {
    auto old_handler = event_handler_;
    context_->Post([old_handler] {
      old_handler(api::Error(YOGI_ERR_CANCELED), kNoEvent, api::kSuccess, {},
                  {});
    });
  }

  observed_events_ = events;
  event_handler_ = handler;
}

void ConnectionManager::CancelAwaitEvent() { AwaitEvent(kNoEvent, {}); }

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
  auto socket = MakeSocketAndKeepItAlive();
  auto weak_socket = utils::TimedTcpSocketWeakPtr(socket);

  auto weak_self = std::weak_ptr<ConnectionManager>{shared_from_this()};
  socket->Accept(&acceptor_, [weak_self, weak_socket](auto& err) {
    auto self = weak_self.lock();
    if (!self) return;

    auto socket = self->StopKeepingSocketAlive(weak_socket);
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

  auto socket = MakeSocketAndKeepItAlive();
  auto weak_socket = utils::TimedTcpSocketWeakPtr(socket);
  socket->Connect(ep, [this, entry, weak_socket](auto& err) {
    auto socket = this->StopKeepingSocketAlive(weak_socket);
    this->OnConnectFinished(err, entry, socket);
  });

  EmitBranchEvent(kBranchDiscoveredEvent, api::kSuccess, adv_uuid, [&] {
    return nlohmann::json{
        {"uuid", boost::uuids::to_string(adv_uuid)},
        {"tcp_server_address", utils::MakeIpAddressString(ep)},
        {"tcp_server_port", ep.port()}};
  });
}

void ConnectionManager::OnConnectFinished(const api::Error& err,
                                          ConnectionsMapEntry* entry,
                                          utils::TimedTcpSocketPtr socket) {
  YOGI_ASSERT(entry != nullptr && entry->first != boost::uuids::uuid{});

  std::lock_guard<std::mutex> lock(connections_mutex_);

  if (err && !entry->second) {
    auto& adv_uuid = entry->first;
    EmitBranchEvent(kBranchQueriedEvent, err, adv_uuid);

    connections_.erase(adv_uuid);  // pointer "entry" is invalid after this

    return;
  }

  StartExchangeBranchInfo(socket, entry);
}

void ConnectionManager::StartExchangeBranchInfo(utils::TimedTcpSocketPtr socket,
                                                ConnectionsMapEntry* entry) {
  auto conn = MakeConnectionAndKeepItAlive(socket);
  auto weak_conn = BranchConnectionWeakPtr(conn);
  conn->ExchangeBranchInfo([this, weak_conn, entry](auto& err) {
    YOGI_ASSERT(weak_conn.lock());
    this->OnExchangeBranchInfoFinished(err, weak_conn.lock(), entry);
    this->StopKeepingConnectionAlive(weak_conn);
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

  auto remote_info = conn->GetRemoteBranchInfo();
  if (blacklisted_uuids_.count(remote_info->GetUuid())) {
    return;
  }

  // UUIDs from advertising message and info message don't match
  CheckAndFixUuidMismatch(remote_info->GetUuid(), &entry);
  YOGI_ASSERT(entry != nullptr);

  if (DoesHigherPriorityConnectionExist(*entry)) {
    return;
  }

  EmitBranchEvent(kBranchQueriedEvent, api::kSuccess, entry->first,
                  [&] { return remote_info->ToJson(); });

  auto res = CheckRemoteBranchInfo(remote_info);
  if (res != api::kSuccess) {
    EmitBranchEvent(kConnectFinishedEvent, res, entry->first);

    blacklisted_uuids_.insert(entry->first);
    connections_.erase(entry->first);
    return;
  }

  entry->second = conn;
  StartAuthenticate(conn);
}

void ConnectionManager::PublishExchangeBranchInfoError(
    const api::Error& err, BranchConnectionPtr conn,
    ConnectionsMapEntry* entry) {
  if (entry) {  // Connect operation failed
    EmitBranchEvent(kBranchQueriedEvent, err, entry->first);
  } else {
    YOGI_LOG_WARNING(
        logger_, info_ << " Exchanging branch info with "
                       << utils::MakeIpAddressString(conn->GetRemoteEndpoint())
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

api::Error ConnectionManager::CheckRemoteBranchInfo(
    const BranchInfoPtr& remote_info) {
  if (remote_info->GetNetName() != info_->GetNetName()) {
    return api::Error(YOGI_ERR_NET_NAME_MISMATCH);
  }

  if (remote_info->GetName() == info_->GetName()) {
    return api::Error(YOGI_ERR_DUPLICATE_BRANCH_NAME);
  }

  if (remote_info->GetPath() == info_->GetPath()) {
    return api::Error(YOGI_ERR_DUPLICATE_BRANCH_PATH);
  }

  for (auto& entry : connections_) {
    if (!entry.second) continue;
    auto branch_info = entry.second->GetRemoteBranchInfo();
    if (branch_info == remote_info) continue;

    if (remote_info->GetName() == branch_info->GetName()) {
      return api::Error(YOGI_ERR_DUPLICATE_BRANCH_NAME);
    }

    if (remote_info->GetPath() == branch_info->GetPath()) {
      return api::Error(YOGI_ERR_DUPLICATE_BRANCH_PATH);
    }
  }

  return api::kSuccess;
}

void ConnectionManager::StartAuthenticate(BranchConnectionPtr conn) {
  auto weak_conn = BranchConnectionWeakPtr(conn);
  conn->Authenticate(password_hash_, [this, weak_conn](auto& err) {
    YOGI_ASSERT(weak_conn.lock());
    this->OnAuthenticateFinished(err, weak_conn.lock());
  });
}

void ConnectionManager::OnAuthenticateFinished(const api::Error& err,
                                               BranchConnectionPtr conn) {
  auto& uuid = conn->GetRemoteBranchInfo()->GetUuid();

  if (err) {
    if (err == api::Error(YOGI_ERR_PASSWORD_MISMATCH)) {
      blacklisted_uuids_.insert(uuid);
    }

    std::lock_guard<std::mutex> lock(connections_mutex_);
    connections_.erase(uuid);

    EmitBranchEvent(kConnectFinishedEvent, err, uuid);
  } else {
    StartSession(conn);
  }
}

void ConnectionManager::StartSession(BranchConnectionPtr conn) {
  auto weak_conn = BranchConnectionWeakPtr(conn);
  conn->RunSession([this, weak_conn](auto& err) {
    YOGI_ASSERT(weak_conn.lock());
    this->OnSessionTerminated(err, weak_conn.lock());
  });

  EmitBranchEvent(kConnectFinishedEvent, api::kSuccess,
                  conn->GetRemoteBranchInfo()->GetUuid());

  connection_changed_handler_(api::kSuccess, conn);
}

void ConnectionManager::OnSessionTerminated(const api::Error& err,
                                            BranchConnectionPtr conn) {
  EmitBranchEvent(kConnectionLostEvent, err,
                  conn->GetRemoteBranchInfo()->GetUuid());

  std::lock_guard<std::mutex> lock(connections_mutex_);
  connections_.erase(conn->GetRemoteBranchInfo()->GetUuid());

  connection_changed_handler_(err, conn);
}

utils::TimedTcpSocketPtr ConnectionManager::MakeSocketAndKeepItAlive() {
  auto socket =
      std::make_shared<utils::TimedTcpSocket>(context_, info_->GetTimeout());
  sockets_kept_alive_.insert(socket);
  return socket;
}

utils::TimedTcpSocketPtr ConnectionManager::StopKeepingSocketAlive(
    const utils::TimedTcpSocketWeakPtr& weak_socket) {
  auto socket = weak_socket.lock();
  YOGI_ASSERT(socket);
  sockets_kept_alive_.erase(socket);
  return socket;
}

BranchConnectionPtr ConnectionManager::MakeConnectionAndKeepItAlive(
    utils::TimedTcpSocketPtr socket) {
  auto conn = std::make_shared<BranchConnection>(socket, info_);
  connections_kept_alive_.insert(conn);
  return conn;
}

BranchConnectionPtr ConnectionManager::StopKeepingConnectionAlive(
    const BranchConnectionWeakPtr& weak_conn) {
  auto conn = weak_conn.lock();
  YOGI_ASSERT(conn);
  connections_kept_alive_.erase(conn);
  return conn;
}

template <typename Fn>
void ConnectionManager::EmitBranchEvent(BranchEvents event,
                                        const api::Error& ev_res,
                                        const boost::uuids::uuid& uuid,
                                        Fn make_json_fn) {
  LogBranchEvent(event, ev_res, make_json_fn);

  std::lock_guard<std::recursive_mutex> lock(event_mutex_);

  if (!event_handler_) return;
  if (!(observed_events_ & event)) return;

  auto handler = event_handler_;
  event_handler_ = {};
  handler(api::kSuccess, event, ev_res, uuid, make_json_fn().dump());
}

void ConnectionManager::EmitBranchEvent(BranchEvents event,
                                        const api::Error& ev_res,
                                        const boost::uuids::uuid& uuid) {
  return EmitBranchEvent(event, ev_res, uuid, [&] {
    return nlohmann::json{{"uuid", boost::uuids::to_string(uuid)}};
  });
}

template <typename Fn>
void ConnectionManager::LogBranchEvent(BranchEvents event,
                                       const api::Error& ev_res,
                                       Fn make_json_fn) {
  switch (event) {
    case kNoEvent:
    case kAllEvents:
      break;

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
