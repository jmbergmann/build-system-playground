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

#include "branch_connection.h"
#include "../../../api/constants.h"
#include "../../../utils/crypto.h"
#include "../../../network/serialize.h"

namespace objects {
namespace detail {

const LoggerPtr BranchConnection::logger_ =
    Logger::CreateStaticInternalLogger("Branch Connection");

BranchConnection::BranchConnection(network::TimedTcpSocketPtr socket,
                                   BranchInfoPtr local_info)
    : socket_(socket),
      context_(socket->GetContext()),
      local_info_(local_info),
      connected_since_(utils::Timestamp::Now()),
      heartbeat_msg_(utils::MakeSharedByteVector(utils::ByteVector{0})),
      ack_msg_(utils::MakeSharedByteVector(utils::ByteVector{0x55})),
      session_started_(false),
      heartbeat_timer_(context_->IoContext()),
      next_result_(api::kSuccess) {}

std::string BranchConnection::MakeInfoString() const {
  auto json = remote_info_->ToJson();
  json["connected_since_"] = connected_since_.ToJavaScriptString();
  return json.dump();
}

void BranchConnection::ExchangeBranchInfo(CompletionHandler handler) {
  YOGI_ASSERT(!remote_info_);

  socket_->Send(local_info_->MakeInfoMessage(), [this, handler](auto& res) {
    if (res.IsError()) {
      handler(res);
    } else {
      this->OnInfoSent(handler);
    }
  });
}

void BranchConnection::Authenticate(utils::SharedByteVector password_hash,
                                    CompletionHandler handler) {
  YOGI_ASSERT(remote_info_);

  if (!CheckNextResult(handler)) return;

  auto my_challenge =
      utils::MakeSharedByteVector(utils::GenerateRandomBytes(8));
  socket_->Send(my_challenge,
                [this, my_challenge, password_hash, handler](auto& res) {
                  if (res.IsError()) {
                    handler(res);
                  } else {
                    this->OnChallengeSent(my_challenge, password_hash, handler);
                  }
                });
}

void BranchConnection::RunSession(CompletionHandler handler) {
  YOGI_ASSERT(remote_info_);
  YOGI_ASSERT(!session_started_);

  if (!CheckNextResult(handler)) return;

  RestartHeartbeatTimer();
  StartReceive();
  session_started_ = true;
  session_completion_handler_ = handler;
}

void BranchConnection::OnInfoSent(CompletionHandler handler) {
  socket_->ReceiveExactly(
      BranchInfo::kInfoMessageHeaderSize,
      [this, handler](const auto& res, const auto& info_msg_hdr) {
        if (res.IsError()) {
          handler(res);
        } else {
          this->OnInfoHeaderReceived(info_msg_hdr, handler);
        }
      });
}

void BranchConnection::OnInfoHeaderReceived(
    const utils::ByteVector& info_msg_hdr, CompletionHandler handler) {
  std::size_t body_size;
  auto res =
      BranchInfo::DeserializeInfoMessageBodySize(&body_size, info_msg_hdr);
  if (res.IsError()) {
    handler(res);
    return;
  }

  if (body_size > api::kMaxMessagePayloadSize) {
    handler(api::Error(YOGI_ERR_PAYLOAD_TOO_LARGE));
    return;
  }

  socket_->ReceiveExactly(
      body_size, [this, info_msg_hdr, handler](auto& res, auto& info_msg_body) {
        if (res.IsError()) {
          handler(res);
        } else {
          this->OnInfoBodyReceived(info_msg_hdr, info_msg_body, handler);
        }
      });
}

void BranchConnection::OnInfoBodyReceived(
    const utils::ByteVector& info_msg_hdr,
    const utils::ByteVector& info_msg_body, CompletionHandler handler) {
  try {
    auto info_msg = info_msg_hdr;
    info_msg.insert(info_msg.end(), info_msg_body.begin(), info_msg_body.end());
    remote_info_ = BranchInfo::CreateFromInfoMessage(
        info_msg, socket_->GetRemoteEndpoint().address());

    if (remote_info_->GetUuid() == local_info_->GetUuid()) {
      throw api::Error(YOGI_ERR_LOOPBACK_CONNECTION);
    }
  } catch (const api::Error& err) {
    handler(err);
    return;
  }

  socket_->Send(ack_msg_, [this, handler](auto& res) {
    if (res.IsError()) {
      handler(res);
    } else {
      this->OnInfoAckSent(handler);
    }
  });
}

void BranchConnection::OnInfoAckSent(CompletionHandler handler) {
  socket_->ReceiveExactly(ack_msg_->size(),
                          [this, handler](auto& res, auto& ack_msg) {
                            this->OnInfoAckReceived(res, ack_msg, handler);
                          });
}

void BranchConnection::OnInfoAckReceived(const api::Result& res,
                                         const utils::ByteVector& ack_msg,
                                         CompletionHandler handler) {
  CheckAckAndSetNextResult(res, ack_msg);
  handler(api::kSuccess);
}

void BranchConnection::OnChallengeSent(utils::SharedByteVector my_challenge,
                                       utils::SharedByteVector password_hash,
                                       CompletionHandler handler) {
  socket_->ReceiveExactly(
      my_challenge->size(), [this, my_challenge, password_hash, handler](
                                auto& res, auto& remote_challenge) {
        if (res.IsError()) {
          handler(res);
        } else {
          this->OnChallengeReceived(remote_challenge, my_challenge,
                                    password_hash, handler);
        }
      });
}

void BranchConnection::OnChallengeReceived(
    const utils::ByteVector& remote_challenge,
    utils::SharedByteVector my_challenge, utils::SharedByteVector password_hash,
    CompletionHandler handler) {
  auto my_solution = SolveChallenge(*my_challenge, *password_hash);
  auto remote_solution = SolveChallenge(remote_challenge, *password_hash);
  socket_->Send(remote_solution, [this, my_solution, handler](auto& res) {
    if (res.IsError()) {
      handler(res);
    } else {
      this->OnSolutionSent(my_solution, handler);
    }
  });
}

utils::SharedByteVector BranchConnection::SolveChallenge(
    const utils::ByteVector& challenge,
    const utils::ByteVector& password_hash) const {
  auto data = challenge;
  data.insert(data.end(), password_hash.begin(), password_hash.end());
  return utils::MakeSharedByteVector(utils::MakeSha256(data));
}

void BranchConnection::OnSolutionSent(utils::SharedByteVector my_solution,
                                      CompletionHandler handler) {
  socket_->ReceiveExactly(
      my_solution->size(),
      [this, my_solution, handler](auto& res, auto& received_solution) {
        if (res.IsError()) {
          handler(res);
        } else {
          this->OnSolutionReceived(received_solution, my_solution, handler);
        }
      });
}

void BranchConnection::OnSolutionReceived(
    const utils::ByteVector& received_solution,
    utils::SharedByteVector my_solution, CompletionHandler handler) {
  bool solutions_match = received_solution == *my_solution;
  socket_->Send(ack_msg_, [this, solutions_match, handler](auto& res) {
    if (res.IsError()) {
      handler(res);
    } else {
      this->OnSolutionAckSent(solutions_match, handler);
    }
  });
}

void BranchConnection::OnSolutionAckSent(bool solutions_match,
                                         CompletionHandler handler) {
  socket_->ReceiveExactly(ack_msg_->size(), [this, solutions_match, handler](
                                                auto& res, auto& ack_msg) {
    this->OnSolutionAckReceived(res, solutions_match, ack_msg, handler);
  });
}

void BranchConnection::OnSolutionAckReceived(const api::Result& res,
                                             bool solutions_match,
                                             const utils::ByteVector& ack_msg,
                                             CompletionHandler handler) {
  CheckAckAndSetNextResult(res, ack_msg);

  if (!solutions_match) {
    handler(api::Error(YOGI_ERR_PASSWORD_MISMATCH));
  } else {
    handler(api::kSuccess);
  }
}

void BranchConnection::RestartHeartbeatTimer() {
  YOGI_ASSERT((remote_info_->GetTimeout() / 2).count() > 0);
  heartbeat_timer_.expires_from_now(remote_info_->GetTimeout() / 2);

  auto weak_self = std::weak_ptr<BranchConnection>(shared_from_this());
  heartbeat_timer_.async_wait([weak_self](auto& ec) {
    if (ec == boost::asio::error::operation_aborted) return;

    auto self = weak_self.lock();
    if (!self) return;

    self->OnHeartbeatTimerExpired();
  });
}

void BranchConnection::OnHeartbeatTimerExpired() {
  auto weak_self = std::weak_ptr<BranchConnection>(shared_from_this());
  socket_->Send(heartbeat_msg_, [weak_self](auto& res) {
    auto self = weak_self.lock();
    if (!self) return;

    if (res.IsError()) {
      self->OnSessionError(res.ToError());
    } else {
      self->RestartHeartbeatTimer();
    }
  });
}

void BranchConnection::StartReceive() {
  // TODO: make this properly without ReceiveExactly and stuff
  auto weak_self = std::weak_ptr<BranchConnection>(shared_from_this());
  socket_->ReceiveExactly(1, [weak_self](auto& res, auto& /*data*/) {
    auto self = weak_self.lock();
    if (!self) return;

    if (res.IsError()) {
      self->OnSessionError(res.ToError());
    } else {
      self->StartReceive();
    }
  });
}

void BranchConnection::OnSessionError(const api::Error& err) {
  heartbeat_timer_.cancel();
  session_completion_handler_(err);
}

void BranchConnection::CheckAckAndSetNextResult(
    const api::Result& res, const utils::ByteVector& ack_msg) {
  if (res.IsError()) {
    next_result_ = res;
  } else if (ack_msg != *ack_msg_) {
    next_result_ = api::Error(YOGI_ERR_DESERIALIZE_MSG_FAILED);
  }
}

bool BranchConnection::CheckNextResult(CompletionHandler handler) {
  if (next_result_.IsError()) {
    auto res = next_result_;
    context_->Post([=] { handler(res); });

    return false;
  }

  return true;
}

}  // namespace detail
}  // namespace objects
