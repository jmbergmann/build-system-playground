#include "branch_connection.h"
#include "../../../utils/crypto.h"
#include "../../../utils/serialize.h"

namespace objects {
namespace detail {

const LoggerPtr BranchConnection::logger_ =
    Logger::CreateStaticInternalLogger("Branch Connection");

BranchConnection::BranchConnection(utils::TimedTcpSocketPtr socket,
                                   BranchInfoPtr local_info)
    : socket_(socket),
      context_(socket->GetContext()),
      local_info_(local_info),
      connected_since_(utils::Timestamp::Now()),
      heartbeat_msg_(utils::MakeSharedByteVector(utils::ByteVector{0})),
      session_started_(false),
      heartbeat_timer_(context_->IoContext()) {}

std::string BranchConnection::MakeInfoString() const {
  auto json = remote_info_->ToJson();
  json["connected_since_"] = connected_since_.ToJavaScriptString();
  return json.dump();
}

void BranchConnection::ExchangeBranchInfo(CompletionHandler handler) {
  YOGI_ASSERT(!remote_info_);

  socket_->Send(local_info_->MakeInfoMessage(),
                [this, handler](const auto& err) {
                  if (err) {
                    handler(err);
                  } else {
                    this->OnInfoSent(handler);
                  }
                });
}

void BranchConnection::Authenticate(utils::SharedByteVector password_hash,
                                    CompletionHandler handler) {
  YOGI_ASSERT(remote_info_);

  auto my_challenge =
      utils::MakeSharedByteVector(utils::GenerateRandomBytes(8));
  socket_->Send(my_challenge,
                [this, my_challenge, password_hash, handler](const auto& err) {
                  if (err) {
                    handler(err);
                  } else {
                    this->OnChallengeSent(my_challenge, password_hash, handler);
                  }
                });
}

void BranchConnection::RunSession(CompletionHandler handler) {
  YOGI_ASSERT(remote_info_);
  YOGI_ASSERT(!session_started_);

  RestartHeartbeatTimer();
  StartReceive();
  session_started_ = true;
  session_completion_handler_ = handler;
}

void BranchConnection::OnInfoSent(CompletionHandler handler) {
  socket_->ReceiveExactly(
      BranchInfo::kInfoMessageHeaderSize,
      [this, handler](const auto& err, const auto& info_msg_hdr) {
        if (err) {
          handler(err);
        } else {
          this->OnInfoHeaderReceived(info_msg_hdr, handler);
        }
      });
}

void BranchConnection::OnInfoHeaderReceived(
    const utils::ByteVector& info_msg_hdr, CompletionHandler handler) {
  std::size_t body_size;
  if (auto err = BranchInfo::DeserializeInfoMessageBodySize(&body_size,
                                                            info_msg_hdr)) {
    handler(err);
    return;
  }

  socket_->ReceiveExactly(
      body_size, [this, info_msg_hdr, handler](const auto& err,
                                               const auto& info_msg_body) {
        if (err) {
          handler(err);
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
  }

  handler(api::kSuccess);
}

void BranchConnection::OnChallengeSent(utils::SharedByteVector my_challenge,
                                       utils::SharedByteVector password_hash,
                                       CompletionHandler handler) {
  socket_->ReceiveExactly(
      my_challenge->size(), [this, my_challenge, password_hash, handler](
                                const auto& err, const auto& remote_challenge) {
        if (err) {
          handler(err);
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
  socket_->Send(remote_solution, [this, my_solution, handler](auto& err) {
    if (err) {
      handler(err);
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
      my_solution->size(), [this, my_solution, handler](
                               const auto& err, const auto& received_solution) {
        if (err) {
          handler(err);
        } else {
          this->OnSolutionReceived(received_solution, my_solution, handler);
        }
      });
}

void BranchConnection::OnSolutionReceived(
    const utils::ByteVector& received_solution,
    utils::SharedByteVector my_solution, CompletionHandler handler) {
  if (received_solution == *my_solution) {
    handler(api::kSuccess);
  } else {
    handler(api::Error(YOGI_ERR_PASSWORD_MISMATCH));
  }
}

void BranchConnection::RestartHeartbeatTimer() {
  YOGI_ASSERT((remote_info_->GetTimeout() / 2).count() > 0);
  heartbeat_timer_.expires_from_now(remote_info_->GetTimeout() / 2);

  auto weak_self = std::weak_ptr<BranchConnection>(shared_from_this());
  heartbeat_timer_.async_wait([weak_self](const auto& ec) {
    if (ec == boost::asio::error::operation_aborted) return;

    auto self = weak_self.lock();
    if (!self) return;

    self->OnHeartbeatTimerExpired();
  });
}

void BranchConnection::OnHeartbeatTimerExpired() {
  auto weak_self = std::weak_ptr<BranchConnection>(shared_from_this());
  socket_->Send(heartbeat_msg_, [weak_self](auto& err) {
    auto self = weak_self.lock();
    if (!self) return;

    if (err) {
      self->OnSessionError(err);
    } else {
      self->RestartHeartbeatTimer();
    }
  });
}

void BranchConnection::StartReceive() {
  // TODO: make this properly without ReceiveExactly and stuff
  auto weak_self = std::weak_ptr<BranchConnection>(shared_from_this());
  socket_->ReceiveExactly(1, [weak_self](auto& err, auto& data) {
    auto self = weak_self.lock();
    if (!self) return;

    if (err) {
      self->OnSessionError(err);
    } else {
      self->StartReceive();
    }
  });
}

void BranchConnection::OnSessionError(const api::Error& err) {
  heartbeat_timer_.cancel();
  session_completion_handler_(err);
}

}  // namespace detail
}  // namespace objects
