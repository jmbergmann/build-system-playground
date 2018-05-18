#include "tcp_base.h"
#include "../../../utils/serialize.h"

#include <boost/uuid/uuid_io.hpp>

namespace objects {
namespace detail {

const LoggerPtr TcpBase::logger_ = Logger::CreateStaticInternalLogger("Branch");

TcpBase::TcpBase(ContextPtr context, LocalBranchInfoPtr info,
                 SuccessHandler&& success_handler, ErrorHandler&& error_handler)
    : context_(context),
      info_(info),
      success_handler_(success_handler),
      error_handler_(error_handler) {}

void TcpBase::Start() { serialized_info_ = info_->MakeInfoMessage(); }

void TcpBase::StartInfoExchange(utils::TimedTcpSocketPtr socket) {
  SendInfo(socket);
}

void TcpBase::CallErrorHandler(const api::Error& err,
                               utils::TimedTcpSocketPtr socket) {
  error_handler_(err, socket);
}

void TcpBase::SendInfo(utils::TimedTcpSocketPtr socket) {
  auto weak_self = std::weak_ptr<TcpBase>{shared_from_this()};
  socket->Send(boost::asio::buffer(serialized_info_),
               [weak_self, socket](const auto& err) {
                 auto self = weak_self.lock();
                 if (!self) return;

                 if (err) {
                   self->error_handler_(err, socket);
                 } else {
                   self->OnInfoSent(socket);
                 }
               });
}

void TcpBase::OnInfoSent(utils::TimedTcpSocketPtr socket) {
  StartReceiveInfoHeader(socket);
}

void TcpBase::StartReceiveInfoHeader(utils::TimedTcpSocketPtr socket) {
  auto weak_self = std::weak_ptr<TcpBase>{shared_from_this()};
  socket->ReceiveExactly(
      BranchInfo::GetInfoMessageHeaderSize(),
      [weak_self, socket](const auto& err, const auto& buffer) {
        auto self = weak_self.lock();
        if (!self) return;

        if (err) {
          self->error_handler_(err, socket);
        } else {
          self->OnInfoHeaderReceived(socket, buffer);
        }
      });
}

void TcpBase::OnInfoHeaderReceived(utils::TimedTcpSocketPtr socket,
                                   const std::vector<char>& buffer) {
  if (auto err = BranchInfo::CheckAdvertisingMessageValidity(buffer)) {
    error_handler_(err, socket);
    return;
  }

  auto branch =
      RemoteBranchInfo::CreateFromAdvertisingMessage(context_, buffer, socket);

  auto it = buffer.begin() + BranchInfo::GetAdvertisingMessageSize();
  std::size_t body_size;
  utils::Deserialize(&body_size, buffer, &it);

  StartReceiveInfoBody(branch, body_size);
}

void TcpBase::StartReceiveInfoBody(RemoteBranchInfoPtr branch,
                                   std::size_t body_size) {
  auto weak_self = std::weak_ptr<TcpBase>{shared_from_this()};
  branch->socket->ReceiveExactly(
      body_size, [weak_self, branch](const auto& err, const auto& buffer) {
        auto self = weak_self.lock();
        if (!self) return;

        if (err) {
          self->error_handler_(err, branch->socket);
        } else {
          self->OnInfoBodyReceived(branch, buffer);
        }
      });
}

void TcpBase::OnInfoBodyReceived(RemoteBranchInfoPtr branch,
                                 const std::vector<char>& buffer) {
  if (!branch->DeserializeInfoMessageBody(buffer)) {
    error_handler_(api::Error(YOGI_ERR_DESERIALIZE_MSG_FAILED), branch->socket);
    return;
  }

  success_handler_(branch);
}

}  // namespace detail
}  // namespace objects
