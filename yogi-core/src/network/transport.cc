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

#include "transport.h"

namespace network {

Transport::Transport(objects::ContextPtr context,
                     std::chrono::nanoseconds timeout,
                     bool created_from_incoming_conn_req,
                     std::string peer_description)
    : context_(context),
      timeout_(timeout),
      created_from_incoming_conn_req_(created_from_incoming_conn_req),
      peer_description_(peer_description),
      tx_timer_(context->IoContext()),
      rx_timer_(context->IoContext()),
      timed_out_(false) {}

Transport::~Transport() {}

void Transport::SendSome(boost::asio::const_buffer data,
                         TransferSomeHandler handler) {
  YOGI_ASSERT(data.size() > 0);

  auto weak_self = MakeWeakPtr();
  StartTimeout(&tx_timer_, weak_self);

  WriteSome(data, [=](auto& res, auto bytes_written) {
    auto self = weak_self.lock();
    if (!self) {
      handler(api::Error(YOGI_ERR_CANCELED), bytes_written);
      return;
    }

    self->tx_timer_.cancel();

    if (self->timed_out_) {
      handler(api::Error(YOGI_ERR_TIMEOUT), bytes_written);
    } else {
      if (res.IsError()) {
        self->Close();
      }

      handler(res, bytes_written);
    }
  });
}

void Transport::SendAll(boost::asio::const_buffer data,
                        TransferAllHandler handler) {
  SendAllImpl(data, api::kSuccess, 0, handler);
}

void Transport::ReceiveSome(boost::asio::mutable_buffer data,
                            TransferSomeHandler handler) {
  YOGI_ASSERT(data.size() > 0);

  auto weak_self = MakeWeakPtr();
  StartTimeout(&rx_timer_, weak_self);

  ReadSome(data, [=](auto& res, auto bytes_read) {
    auto self = weak_self.lock();
    if (!self) {
      handler(api::Error(YOGI_ERR_CANCELED), bytes_read);
      return;
    }

    self->rx_timer_.cancel();

    if (self->timed_out_) {
      handler(api::Error(YOGI_ERR_TIMEOUT), bytes_read);
    } else {
      if (res.IsError()) {
        self->Close();
      }

      handler(res, bytes_read);
    }
  });
}

void Transport::ReceiveAll(boost::asio::mutable_buffer data,
                           TransferAllHandler handler) {
  ReceiveAllImpl(data, api::kSuccess, 0, handler);
}

void Transport::SendAllImpl(boost::asio::const_buffer data,
                            const api::Result& res, std::size_t bytes_written,
                            TransferAllHandler handler) {
  YOGI_ASSERT(data.size() > 0);

  if (res.IsSuccess() && bytes_written < data.size()) {
    data += bytes_written;
    this->SendSome(data, [=](auto& res, auto bytes_written) {
      this->SendAllImpl(data, res, bytes_written, handler);
    });
  } else {
    handler(res);
  }
}

void Transport::Close() {
  Shutdown();
  YOGI_DEBUG_ONLY(close_called_ = true;)
}

void Transport::ReceiveAllImpl(boost::asio::mutable_buffer data,
                               const api::Result& res, std::size_t bytes_read,
                               TransferAllHandler handler) {
  YOGI_ASSERT(data.size() > 0);

  if (res.IsSuccess() && bytes_read < data.size()) {
    data += bytes_read;
    this->ReceiveSome(data, [=](auto& res, auto bytes_read) {
      this->ReceiveAllImpl(data, res, bytes_read, handler);
    });
  } else {
    handler(res);
  }
}

void Transport::StartTimeout(boost::asio::steady_timer* timer,
                             TransportWeakPtr weak_self) {
  timer->expires_from_now(timeout_);
  timer->async_wait([weak_self](auto& ec) {
    auto self = weak_self.lock();
    if (!self) return;

    if (!ec) {
      self->OnTimeout();
    }
  });
}

void Transport::OnTimeout() {
  timed_out_ = true;

  Close();
}

}  // namespace network

std::ostream& operator<<(std::ostream& os,
                         const network::Transport& transport) {
  os << transport.GetPeerDescription();
  return os;
}
