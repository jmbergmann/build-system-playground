#include "tcp_base.h"
#include "../../../utils/serialize.h"

#include <boost/uuid/uuid_io.hpp>

namespace objects {
namespace detail {

const LoggerPtr TcpBase::logger_ = Logger::CreateStaticInternalLogger("Branch");

TcpBase::TcpBase(ContextPtr context, LocalBranchInfoPtr info,
                 ObserverFn&& observer_fn)
    : context_(context), info_(info), observer_fn_(observer_fn) {}

void TcpBase::Start() { serialized_info_ = info_->MakeInfoMessage(); }

void TcpBase::StartInfoExchange(utils::TimedTcpSocketPtr socket) {
  SendInfo(socket);
}

void TcpBase::SendInfo(utils::TimedTcpSocketPtr socket) {
  auto weak_self = std::weak_ptr<TcpBase>{shared_from_this()};
  socket->Send(boost::asio::buffer(serialized_info_), [weak_self, socket](
                                                          const auto& err) {
    auto self = weak_self.lock();
    if (!self) return;

    if (err) {
      YOGI_LOG_ERROR(self->logger_, "Could not send branch info to "
                                        << *socket << ": " << err);
    } else {
      self->OnInfoSent(socket);
    }
  });
}

void TcpBase::OnInfoSent(utils::TimedTcpSocketPtr socket) {
  YOGI_LOG_TRACE(logger_, "Local branch info sent to " << *socket);
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
          YOGI_LOG_ERROR(self->logger_,
                         "Could not receive branch info header from "
                             << *socket << ": " << err);
        } else {
          self->OnInfoHeaderReceived(socket, buffer);
        }
      });
}

void TcpBase::OnInfoHeaderReceived(utils::TimedTcpSocketPtr socket,
                                   const std::vector<char>& buffer) {
  auto branch = RemoteBranchInfo::CreateFromAdvertisingMessage(buffer, socket);

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
          YOGI_LOG_ERROR(self->logger_,
                         "Could not receive branch info body from "
                             << *branch->socket << ": " << err);
        } else {
          self->OnInfoBodyReceived(branch, buffer);
        }
      });
}

void TcpBase::OnInfoBodyReceived(RemoteBranchInfoPtr branch,
                                 const std::vector<char>& buffer) {
  if (!branch->DeserializeInfoMessageBody(buffer)) {
    return;
  }

  YOGI_LOG_INFO(logger_, "Received branch info for " << branch->uuid << " from "
                                                     << *branch->socket);

  observer_fn_(std::move(branch));
}

}  // namespace detail
}  // namespace objects
