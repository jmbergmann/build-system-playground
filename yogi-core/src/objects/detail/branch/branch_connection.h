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

#pragma once

#include "../../../config.h"
#include "../../../network/msg_transport.h"
#include "../../context.h"
#include "../../logger.h"
#include "branch_info.h"

#include <boost/asio.hpp>
#include <functional>
#include <memory>
#include <atomic>

namespace objects {
namespace detail {

class BranchConnection;
typedef std::shared_ptr<BranchConnection> BranchConnectionPtr;
typedef std::weak_ptr<BranchConnection> BranchConnectionWeakPtr;

class BranchConnection : public std::enable_shared_from_this<BranchConnection> {
 public:
  typedef std::function<void(const api::Result&)> CompletionHandler;

  BranchConnection(network::TransportPtr transport,
                   const boost::asio::ip::address& peer_address,
                   LocalBranchInfoPtr local_info);

  BranchInfoPtr GetRemoteBranchInfo() const { return remote_info_; }
  std::string MakeInfoString() const;
  bool SessionStarted() const { return session_started_; }

  bool CreatedFromIncomingConnectionRequest() const {
    return transport_->CreatedFromIncomingConnectionRequest();
  };

  const std::string& GetPeerDescription() const {
    return transport_->GetPeerDescription();
  }

  void ExchangeBranchInfo(CompletionHandler handler);
  void Authenticate(utils::SharedByteVector password_hash,
                    CompletionHandler handler);
  void RunSession(CompletionHandler handler);

 private:
  BranchConnectionWeakPtr MakeWeakPtr() { return {shared_from_this()}; }
  void OnInfoSent(CompletionHandler handler);
  void OnInfoHeaderReceived(utils::SharedByteVector buffer,
                            CompletionHandler handler);
  void OnInfoBodyReceived(utils::SharedByteVector info_msg,
                          CompletionHandler handler);
  void OnInfoAckSent(CompletionHandler handler);
  void OnInfoAckReceived(const api::Result& res,
                         utils::SharedByteVector ack_msg,
                         CompletionHandler handler);
  void OnChallengeSent(utils::SharedByteVector my_challenge,
                       utils::SharedByteVector password_hash,
                       CompletionHandler handler);
  void OnChallengeReceived(utils::SharedByteVector remote_challenge,
                           utils::SharedByteVector my_challenge,
                           utils::SharedByteVector password_hash,
                           CompletionHandler handler);
  utils::SharedByteVector SolveChallenge(
      const utils::ByteVector& challenge,
      const utils::ByteVector& password_hash) const;
  void OnSolutionSent(utils::SharedByteVector my_solution,
                      CompletionHandler handler);
  void OnSolutionReceived(utils::SharedByteVector received_solution,
                          utils::SharedByteVector my_solution,
                          CompletionHandler handler);
  void OnSolutionAckSent(bool solutions_match, CompletionHandler handler);
  void OnSolutionAckReceived(const api::Result& res, bool solutions_match,
                             utils::SharedByteVector ack_msg,
                             CompletionHandler handler);
  void RestartHeartbeatTimer();
  void OnHeartbeatTimerExpired();
  void StartReceive();
  void OnSessionError(const api::Error& err);
  void CheckAckAndSetNextResult(const api::Result& res,
                                const utils::ByteVector& ack_msg);
  bool CheckNextResult(CompletionHandler handler);

  static const LoggerPtr logger_;

  const network::TransportPtr transport_;
  const objects::ContextPtr context_;
  const LocalBranchInfoPtr local_info_;
  const boost::asio::ip::address peer_address_;
  const utils::Timestamp connected_since_;
  const utils::ByteVector heartbeat_msg_;
  const utils::SharedByteVector ack_msg_;
  RemoteBranchInfoPtr remote_info_;
  utils::SharedByteVector received_msg_;
  network::MessageTransportPtr msg_transport_;
  std::atomic<bool> session_started_;
  CompletionHandler session_completion_handler_;
  boost::asio::steady_timer heartbeat_timer_;
  api::Result next_result_;
};

}  // namespace detail
}  // namespace objects
