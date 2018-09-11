/*
 * This file is part of the Yogi distribution https://github.com/yohummus/yogi.
 * Copyright (c) 2018 Johannes Bergmann.
 *
 * This library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library. If not, see <http://www.gnu.org/licenses/>.
 */

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
  if (adv_ep.port() == 0) {
    throw api::Error(YOGI_ERR_INVALID_PARAM);
  }

  using namespace boost::asio::ip;
  SetupAcceptor(adv_ep.protocol() == udp::v4() ? tcp::v4() : tcp::v6());
}

ConnectionManager::~ConnectionManager() { CancelAwaitEvent(); }

void ConnectionManager::Start(BranchInfoPtr info) {
  info_ = info;
  StartAccept();
  adv_sender_->Start(info);
  adv_receiver_->Start(info);

  YOGI_LOG_DEBUG(logger_,
                 info_ << " Started ConnectionManager with TCP server port "
                       << info_->GetTcpEndpoint().port()
                       << (info_->GetGhostMode() ? " in ghost mode" : ""));
}

ConnectionManager::BranchInfoStringsList
ConnectionManager::MakeConnectedBranchesInfoStrings() const {
  BranchInfoStringsList branches;

  std::lock_guard<std::mutex> lock(connections_mutex_);
  for (auto& entry : connections_) {
    YOGI_ASSERT(entry.second);
    if (entry.second->SessionStarted()) {
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

  YOGI_LOG_DEBUG(logger_, info_ << " Accepted incoming TCP connection from "
                                << utils::MakeIpAddressString(
                                       socket->GetRemoteEndpoint()));

  StartExchangeBranchInfo(socket, {});
  StartAccept();
}

void ConnectionManager::OnAdvertisementReceived(
    const boost::uuids::uuid& adv_uuid,
    const boost::asio::ip::tcp::endpoint& ep) {
  std::lock_guard<std::mutex> lock(connections_mutex_);
  if (connections_.count(adv_uuid)) return;
  if (blacklisted_uuids_.count(adv_uuid)) return;
  if (pending_connects_.count(adv_uuid)) return;

  YOGI_LOG_DEBUG(logger_, info_ << " Attempting to connect to [" << adv_uuid
                                << "] on " << utils::MakeIpAddressString(ep)
                                << ":" << ep.port());

  auto socket = MakeSocketAndKeepItAlive();
  auto weak_socket = utils::TimedTcpSocketWeakPtr(socket);
  socket->Connect(ep, [this, adv_uuid, weak_socket](auto& err) {
    auto socket = this->StopKeepingSocketAlive(weak_socket);
    this->OnConnectFinished(err, adv_uuid, socket);
  });

  EmitBranchEvent(kBranchDiscoveredEvent, api::kSuccess, adv_uuid, [&] {
    return nlohmann::json{
        {"uuid", boost::uuids::to_string(adv_uuid)},
        {"tcp_server_address", utils::MakeIpAddressString(ep)},
        {"tcp_server_port", ep.port()}};
  });
}

void ConnectionManager::OnConnectFinished(const api::Error& err,
                                          const boost::uuids::uuid& adv_uuid,
                                          utils::TimedTcpSocketPtr socket) {
  if (err) {
    EmitBranchEvent(kBranchQueriedEvent, err, adv_uuid);
    pending_connects_.erase(adv_uuid);
    return;
  }

  YOGI_LOG_DEBUG(logger_, info_ << " TCP connection to " << *socket
                                << " established successfully");

  StartExchangeBranchInfo(socket, adv_uuid);
}

void ConnectionManager::StartExchangeBranchInfo(
    utils::TimedTcpSocketPtr socket, const boost::uuids::uuid& adv_uuid) {
  auto conn = MakeConnectionAndKeepItAlive(socket);
  auto weak_conn = BranchConnectionWeakPtr(conn);
  conn->ExchangeBranchInfo([this, weak_conn, adv_uuid](auto& err) {
    YOGI_ASSERT(weak_conn.lock());
    this->OnExchangeBranchInfoFinished(err, weak_conn.lock(), adv_uuid);
    this->StopKeepingConnectionAlive(weak_conn);
    this->pending_connects_.erase(adv_uuid);
  });
}

void ConnectionManager::OnExchangeBranchInfoFinished(
    const api::Error& err, BranchConnectionPtr conn,
    const boost::uuids::uuid& adv_uuid) {
  if (!CheckExchangeBranchInfoError(err, conn)) {
    return;
  }

  auto remote_info = conn->GetRemoteBranchInfo();
  auto& remote_uuid = remote_info->GetUuid();

  if (!conn->SourceIsTcpServer() && !VerifyUuidsMatch(remote_uuid, adv_uuid)) {
    return;
  }

  if (!VerifyUuidNotBlacklisted(remote_uuid)) {
    return;
  }

  YOGI_LOG_DEBUG(logger_,
                 info_ << " Successfully exchanged branch info with "
                       << remote_info << " (source: TCP "
                       << (conn->SourceIsTcpServer() ? "server" : "client")
                       << ")");

  std::lock_guard<std::mutex> lock(connections_mutex_);
  auto res = connections_.insert(std::make_pair(remote_uuid, conn));
  bool conn_already_exists = !res.second;

  if (!VerifyConnectionHasHigherPriority(conn_already_exists, conn)) {
    return;
  }

  res.first->second = conn;

  if (!conn_already_exists) {
    EmitBranchEvent(kBranchQueriedEvent, api::kSuccess, remote_uuid,
                    [&] { return remote_info->ToJson(); });

    // If a connection already exists, the following check has already been
    // performed successfully, so we don't do it again
    if (auto chk_err = CheckRemoteBranchInfo(remote_info)) {
      EmitBranchEvent(kConnectFinishedEvent, chk_err, remote_uuid);
      return;
    }
  }

  if (info_->GetGhostMode()) {
    blacklisted_uuids_.insert(remote_uuid);
  } else {
    StartAuthenticate(conn);
  }
}

bool ConnectionManager::CheckExchangeBranchInfoError(
    const api::Error& err, const BranchConnectionPtr& conn) {
  if (!err) {
    return true;
  }

  if (conn->SourceIsTcpServer()) {
    YOGI_LOG_ERROR(
        logger_,
        info_ << " Exchanging branch info with TCP server connection from "
              << utils::MakeIpAddressString(conn->GetRemoteEndpoint())
              << " failed: " << err);
  } else {
    YOGI_LOG_ERROR(
        logger_,
        info_ << " Exchanging branch info with TCP client connection to "
              << utils::MakeIpAddressString(conn->GetRemoteEndpoint()) << ":"
              << conn->GetRemoteEndpoint().port() << " failed: " << err);
  }

  return false;
}

bool ConnectionManager::VerifyUuidsMatch(const boost::uuids::uuid& remote_uuid,
                                         const boost::uuids::uuid& adv_uuid) {
  if (remote_uuid == adv_uuid) {
    return true;
  }

  YOGI_LOG_WARNING(logger_,
                   info_ << " Dropping connection since branch info UUID ["
                         << remote_uuid << "] does not match advertised UUID ["
                         << adv_uuid
                         << "]. This will likely be fixed with the next "
                            "connection attempt.");
  return false;
}

bool ConnectionManager::VerifyUuidNotBlacklisted(
    const boost::uuids::uuid& uuid) {
  if (!blacklisted_uuids_.count(uuid)) {
    return true;
  }

  YOGI_LOG_DEBUG(logger_, info_ << " Dropping connection to [" << uuid
                                << "] since it is blacklisted");
  return false;
}

bool ConnectionManager::VerifyConnectionHasHigherPriority(
    bool conn_already_exists, const BranchConnectionPtr& conn) {
  if (!conn_already_exists) {
    return true;
  }

  auto& remote_uuid = conn->GetRemoteBranchInfo()->GetUuid();
  YOGI_ASSERT(connections_[remote_uuid]);

  if ((remote_uuid < info_->GetUuid()) == conn->SourceIsTcpServer()) {
    return true;
  }

  YOGI_LOG_DEBUG(
      logger_,
      info_ << " Dropping TCP "
            << (conn->SourceIsTcpServer() ? "server" : "client")
            << " connection to " << conn
            << " since a connection with a higher priority already exists")

  return false;
}

api::Error ConnectionManager::CheckRemoteBranchInfo(
    const BranchInfoPtr& remote_info) {
  if (remote_info->GetNetworkName() != info_->GetNetworkName()) {
    blacklisted_uuids_.insert(remote_info->GetUuid());
    return api::Error(YOGI_ERR_NET_NAME_MISMATCH);
  }

  if (remote_info->GetName() == info_->GetName()) {
    blacklisted_uuids_.insert(remote_info->GetUuid());
    return api::Error(YOGI_ERR_DUPLICATE_BRANCH_NAME);
  }

  if (remote_info->GetPath() == info_->GetPath()) {
    blacklisted_uuids_.insert(remote_info->GetUuid());
    return api::Error(YOGI_ERR_DUPLICATE_BRANCH_PATH);
  }

  for (auto& entry : connections_) {
    auto branch_info = entry.second->GetRemoteBranchInfo();
    if (branch_info == remote_info) continue;

    YOGI_ASSERT(remote_info->GetUuid() != branch_info->GetUuid());

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
    YOGI_LOG_DEBUG(logger_, info_ << " Successfully authenticated with "
                                  << conn->GetRemoteBranchInfo());
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

  YOGI_LOG_DEBUG(logger_, info_ << " Successfully started session for "
                                << conn->GetRemoteBranchInfo());

  connection_changed_handler_(api::kSuccess, conn);
}

void ConnectionManager::OnSessionTerminated(const api::Error& err,
                                            BranchConnectionPtr conn) {
  YOGI_LOG_DEBUG(logger_, info_ << " Session for "
                                << conn->GetRemoteBranchInfo()
                                << " terminated: " << err);

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
