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

#include "msg_transport.h"

namespace network {
namespace internal {

std::size_t CalculateMsgSizeFieldLength(std::size_t msg_size) {
  return 1 + (msg_size >= (1 << 7)) + (msg_size >= (1 << 14)) +
         (msg_size >= (1 << 21)) + (msg_size >= (1 << 28));
}

std::size_t SerializeMsgSizeField(std::size_t msg_size,
                                  std::array<utils::Byte, 5>* buffer) {
  auto length = CalculateMsgSizeFieldLength(msg_size);

  auto it = buffer->begin();
  for (auto i = length; i > 0; --i) {
    auto byte = static_cast<utils::Byte>((msg_size >> ((i - 1) * 7)) & 0x7F);
    byte |= (i > 1 ? (1 << 7) : 0);
    *it++ = byte;
  }

  return length;
}

bool DeserializeMsgSizeField(const std::array<utils::Byte, 5>& buffer,
                             std::size_t size, std::size_t* msg_size) {
  YOGI_ASSERT(size <= buffer.size());

  std::size_t tmp = 0;

  for (std::size_t i = 0; i < size; ++i) {
    auto byte = buffer[i];

    tmp |= static_cast<std::size_t>(byte & ~(1 << 7));
    if (!(byte & (1 << 7))) {
      *msg_size = tmp;
      return true;
    }

    tmp <<= 7;
  }

  return false;
}

}  // namespace internal

MessageTransport::MessageTransport(TransportPtr transport,
                                   std::size_t tx_queue_size,
                                   std::size_t rx_queue_size)
    : context_(transport->GetContext()),
      transport_(transport),
      tx_rb_(tx_queue_size),
      rx_rb_(rx_queue_size),
      last_tx_error_(api::kSuccess),
      last_rx_error_(api::kSuccess),
      send_to_transport_running_(false),
      last_send_oid_(0),
      receive_from_transport_running_(false) {
  ResetReceivedSizeField();
}

void MessageTransport::Start() { ReceiveSomeBytesFromTransport(); }

bool MessageTransport::TrySend(boost::asio::const_buffer msg) {
  std::lock_guard<std::mutex> lock(tx_mutex_);
  if (last_tx_error_.IsError()) {
    throw last_tx_error_.ToError();
  }

  return TrySendImpl(msg);
}

MessageTransport::SendOperationId MessageTransport::SendAsync(
    boost::asio::const_buffer msg, SendHandler handler) {
  std::lock_guard<std::mutex> lock(tx_mutex_);
  auto oid = GetNextSendOperationId();

  if (last_tx_error_.IsError()) {
    transport_->GetContext()->Post([=] { handler(last_tx_error_, oid); });
    return oid;
  }

  if (TrySendImpl(msg)) {
    transport_->GetContext()->Post([=] { handler(api::kSuccess, oid); });
  } else {
    PendingSend ps = {oid, msg, handler};
    pending_sends_.push_back(ps);

    YOGI_ASSERT(send_to_transport_running_);
  }

  return oid;
}

bool MessageTransport::CancelSend(SendOperationId oid) {
  std::lock_guard<std::mutex> lock(tx_mutex_);
  auto it = std::find_if(pending_sends_.begin(), pending_sends_.end(),
                         [&](auto& ps) { return ps.oid == oid; });

  if (it == pending_sends_.end()) return false;

  auto handler = std::move(it->handler);
  pending_sends_.erase(it);

  transport_->GetContext()->Post(
      [=] { handler(api::Error(YOGI_ERR_CANCELED), oid); });

  return true;
}

void MessageTransport::ReceiveAsync(boost::asio::mutable_buffer msg,
                                    ReceiveHandler handler) {
  YOGI_ASSERT(!pending_receive_handler_);
  YOGI_ASSERT(!size_field_valid_);

  if (last_rx_error_.IsError()) {
    transport_->GetContext()->Post([=] { handler(last_rx_error_, 0); });
    return;
  }

  pending_receive_buffer_ = msg;
  pending_receive_handler_ = handler;
  TryDeliveringPendingReceive();

  ReceiveSomeBytesFromTransport();
}

void MessageTransport::CancelReceive() {
  if (!pending_receive_handler_) return;

  ReceiveHandler handler;
  std::swap(handler, pending_receive_handler_);

  transport_->GetContext()->Post(
      [=] { handler(api::Error(YOGI_ERR_CANCELED), 0); });
}

bool MessageTransport::TrySendImpl(boost::asio::const_buffer msg) {
  if (!CanSend(msg.size())) {
    return false;
  }

  SizeFieldBuffer size_field_buf;
  auto n = internal::SerializeMsgSizeField(msg.size(), &size_field_buf);
  auto bytes_written = tx_rb_.Write(size_field_buf.data(), n);
  YOGI_UNUSED(bytes_written);
  YOGI_ASSERT(bytes_written == n);

  bytes_written =
      tx_rb_.Write(static_cast<const utils::Byte*>(msg.data()), msg.size());
  YOGI_ASSERT(bytes_written == msg.size());

  SendSomeBytesToTransport();

  return true;
}

bool MessageTransport::CanSend(std::size_t msg_size) const {
  YOGI_ASSERT(msg_size + internal::CalculateMsgSizeFieldLength(msg_size) <=
              tx_rb_.Capacity());

  auto n = tx_rb_.AvailableForWrite();
  if (n >= msg_size + 5) return true;  // optimisation since this is very likely
  return n >= msg_size + internal::CalculateMsgSizeFieldLength(msg_size);
}

void MessageTransport::SendSomeBytesToTransport() {
  YOGI_ASSERT(!tx_rb_.Empty());

  if (send_to_transport_running_) return;
  send_to_transport_running_ = true;

  auto weak_self = MakeWeakPtr();
  transport_->SendSomeAsync(tx_rb_.FirstReadArray(), [=](auto& res, auto n) {
    auto self = weak_self.lock();
    if (!self) return;

    if (res.IsError()) {
      self->HandleSendError(res.ToError());
      return;
    }

    std::lock_guard<std::mutex> lock(tx_mutex_);
    self->tx_rb_.CommitFirstReadArray(n);
    send_to_transport_running_ = false;

    if (!self->tx_rb_.Empty()) {
      self->SendSomeBytesToTransport();
    }

    self->RetrySendingPendingSends();
  });
}

void MessageTransport::RetrySendingPendingSends() {
  auto it = pending_sends_.begin();
  while (it != pending_sends_.end() && TrySendImpl(it->msg)) {
    auto handler = std::move(it->handler);
    auto oid = it->oid;
    transport_->GetContext()->Post([=] { handler(api::kSuccess, oid); });
    ++it;
  }

  pending_sends_.erase(pending_sends_.begin(), it);
}

bool MessageTransport::TryGetReceivedSizeField(std::size_t* msg_size) {
  if (size_field_valid_) {
    *msg_size = size_field_;
    return true;
  }

  rx_rb_.PopUntil([&](utils::Byte byte) {
    size_field_buffer_[size_field_buffer_size_++] = byte;
    bool ok = internal::DeserializeMsgSizeField(
        size_field_buffer_, size_field_buffer_size_, msg_size);

    if (ok) {
      size_field_ = *msg_size;
      size_field_valid_ = true;

      if (size_field_ > rx_rb_.Capacity()) {
        HandleReceiveError(api::Error(YOGI_ERR_DESERIALIZE_MSG_FAILED));
      }

      return true;
    } else if (size_field_buffer_size_ >= size_field_buffer_.size()) {
      HandleReceiveError(api::Error(YOGI_ERR_DESERIALIZE_MSG_FAILED));
      return true;
    }

    return false;
  });

  return size_field_valid_;
}

void MessageTransport::ResetReceivedSizeField() {
  size_field_buffer_size_ = 0;
  size_field_valid_ = false;
}

void MessageTransport::ReceiveSomeBytesFromTransport() {
  if (receive_from_transport_running_) return;
  receive_from_transport_running_ = true;

  auto weak_self = MakeWeakPtr();
  transport_->ReceiveSomeAsync(rx_rb_.FirstWriteArray(),
                               [=](auto& res, auto n) {
                                 auto self = weak_self.lock();
                                 if (!self) return;

                                 if (res.IsError()) {
                                   self->HandleReceiveError(res.ToError());
                                   return;
                                 }

                                 self->rx_rb_.CommitFirstWriteArray(n);
                                 receive_from_transport_running_ = false;

                                 self->TryDeliveringPendingReceive();

                                 if (!self->rx_rb_.Full()) {
                                   self->ReceiveSomeBytesFromTransport();
                                 }
                               });
}

void MessageTransport::TryDeliveringPendingReceive() {
  if (!pending_receive_handler_) return;

  std::size_t size;
  if (!TryGetReceivedSizeField(&size) || rx_rb_.AvailableForRead() < size) {
    return;
  }

  ReceiveHandler handler;
  std::swap(handler, pending_receive_handler_);
  ResetReceivedSizeField();

  auto n = std::min(size, pending_receive_buffer_.size());
  rx_rb_.Read(static_cast<utils::Byte*>(pending_receive_buffer_.data()), n);

  if (n < size) {
    rx_rb_.Discard(size - n);
    transport_->GetContext()->Post(
        [=] { handler(api::Error(YOGI_ERR_BUFFER_TOO_SMALL), size); });
  } else {
    transport_->GetContext()->Post([=] { handler(api::kSuccess, size); });
  }
}

void MessageTransport::HandleSendError(const api::Error& err) {
  YOGI_LOG_ERROR(logger_, "Sending message failed: " << err);

  Close();

  std::lock_guard<std::mutex> lock(tx_mutex_);
  last_tx_error_ = err;

  for (auto& ps : pending_sends_) {
    ps.handler(err, ps.oid);
  }

  pending_sends_.clear();
}

void MessageTransport::HandleReceiveError(const api::Error& err) {
  YOGI_LOG_ERROR(logger_, "Receiving message failed: " << err);

  Close();

  last_rx_error_ = err;
  if (pending_receive_handler_) {
    ReceiveHandler handler;
    std::swap(handler, pending_receive_handler_);

    transport_->GetContext()->Post([=] { handler(api::Error(err), 0); });
  }
}

MessageTransport::SendOperationId MessageTransport::GetNextSendOperationId() {
  ++last_send_oid_;
  if (last_send_oid_ <= 0) last_send_oid_ = 1;
  return last_send_oid_;
}

const objects::LoggerPtr MessageTransport::logger_ =
    objects::Logger::CreateStaticInternalLogger("MessageTransport");

}  // namespace network
