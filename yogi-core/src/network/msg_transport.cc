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

std::size_t SerializeMsgSizeField(std::size_t msg_size,
                                  std::array<utils::Byte, 5>* buffer) {
  int length = 1 + (msg_size >= (1 << 7)) + (msg_size >= (1 << 14)) +
               (msg_size >= (1 << 21)) + (msg_size >= (1 << 28));

  auto it = buffer->begin();
  for (int i = length; i > 0; --i) {
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
    : transport_(transport),
      tx_rb_(tx_queue_size),
      rx_rb_(rx_queue_size),
      send_to_transport_running_(false) {}

bool MessageTransport::Send(boost::asio::const_buffer msg) {
  std::lock_guard<std::mutex> lock(tx_mutex_);
  return SendImpl(msg);
}

void MessageTransport::SendAsync(boost::asio::const_buffer msg, SendHandler handler) {
  std::lock_guard<std::mutex> lock(tx_mutex_);
  if (SendImpl(msg)) {
    transport_->GetContext()->Post([=]{ handler(api::kSuccess); });
  }
  else {
    PendingSend ps = {msg, handler};
    pending_sends_.push_back(ps);

    YOGI_ASSERT(send_to_transport_running_);
  }
}

void MessageTransport::CancelSend() {
  std::lock_guard<std::mutex> lock(tx_mutex_);
  for (auto& ps : pending_sends_) {
    ps.handler(api::Error(YOGI_ERR_CANCELED));
  }

  pending_sends_.clear();
}

void MessageTransport::ReceiveAsync(boost::asio::mutable_buffer msg,
                               ReceiveHandler handler) {}

void MessageTransport::CancelReceive() {}

bool MessageTransport::SendImpl(boost::asio::const_buffer msg) {
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
  // Note: We use 5 here as the maximum length of the serialized msg_size value
  //       instead of actually calculating the length. Nobody gives a shit about
  //       those few bytes anyway.
  return msg_size + 5 <= tx_rb_.AvailableForWrite();
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
      YOGI_LOG_ERROR(self->logger_,
                     "Sending bytes over transport failed: " << res);
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
  while (it != pending_sends_.end() && SendImpl(it->msg)) {
    auto handler = std::move(it->handler);
    transport_->GetContext()->Post([=] { handler(api::kSuccess); });
    ++it;
  }

  pending_sends_.erase(pending_sends_.begin(), it);
}

const objects::LoggerPtr MessageTransport::logger_ =
    objects::Logger::CreateStaticInternalLogger("MessageTransport");

}  // namespace network
