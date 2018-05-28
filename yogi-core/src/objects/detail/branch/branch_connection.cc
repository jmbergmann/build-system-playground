#include "branch_connection.h"
#include "../../../utils/serialize.h"

namespace objects {
namespace detail {

const LoggerPtr BranchConnection::logger_ =
    Logger::CreateStaticInternalLogger("Branch Connection");

BranchConnection::BranchConnection(utils::TimedTcpSocketPtr socket,
                                   BranchInfoPtr local_info)
    : socket_(socket),
      local_info_(local_info),
      connected_since_(utils::Timestamp::Now()) {}

std::string BranchConnection::MakeInfoString() const {
  auto json = remote_info_->ToJson();
  json["connected_since_"] = connected_since_.ToJavaScriptString();
  return json.dump();
}

void BranchConnection::ExchangeBranchInfo(CompletionHandler handler) {
  YOGI_ASSERT(!remote_info_);

  socket_->Send(boost::asio::buffer(local_info_->MakeInfoMessage()),
                [this, handler](const auto& err) {
                  if (err) {
                    handler(err);
                  } else {
                    this->OnInfoSent(handler);
                  }
                });
}

void BranchConnection::Authenticate(CompletionHandler handler) {
  YOGI_ASSERT(remote_info_);
}

void BranchConnection::RunSession(CompletionHandler handler) {
  YOGI_ASSERT(remote_info_);
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
    const std::vector<char>& info_msg_hdr, CompletionHandler handler) {
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
    const std::vector<char>& info_msg_hdr,
    const std::vector<char>& info_msg_body, CompletionHandler handler) {
  try {
    auto info_msg = info_msg_hdr;
    info_msg.insert(info_msg.end(), info_msg_body.begin(), info_msg_body.end());
    remote_info_ = BranchInfo::CreateFromInfoMessage(
        info_msg, socket_->GetRemoteEndpoint().address());
  } catch (const api::Error& err) {
    handler(err);
  }

  handler(api::kSuccess);
}

}  // namespace detail
}  // namespace objects
