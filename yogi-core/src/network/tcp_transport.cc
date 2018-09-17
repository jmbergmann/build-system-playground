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

#include "tcp_transport.h"
#include "ip.h"

namespace network {

void TcpTransport::Accept(objects::ContextPtr context,
                          boost::asio::ip::tcp::acceptor* acceptor,
                          std::chrono::nanoseconds timeout,
                          CreateHandler handler) {
  auto socket =
      std::make_shared<boost::asio::ip::tcp::socket>(context->IoContext());
  acceptor->async_accept(*socket, [=](auto& ec) {
    if (!ec) {
      auto transport = TcpTransportPtr(
          new TcpTransport(context, std::move(*socket), timeout, true));
      transport->SetNoDelayOption();
      handler(api::kSuccess, transport);
    } else if (ec == boost::asio::error::operation_aborted) {
      handler(api::Error(YOGI_ERR_CANCELED), {});
    } else {
      handler(api::Error(YOGI_ERR_ACCEPT_SOCKET_FAILED), {});
    }
  });
}

void TcpTransport::Connect(objects::ContextPtr context,
                           const boost::asio::ip::tcp::endpoint& ep,
                           std::chrono::nanoseconds timeout,
                           CreateHandler handler) {
  struct ConnectData {
    ConnectData(boost::asio::io_context& ioc) : socket(ioc), timer(ioc) {}

    boost::asio::ip::tcp::socket socket;
    boost::asio::steady_timer timer;
    bool timed_out = false;
  };

  auto condat = std::make_shared<ConnectData>(context->IoContext());

  condat->socket.async_connect(ep, [=](auto& ec) {
    if (condat->timed_out) {
      handler(api::Error(YOGI_ERR_TIMEOUT), {});
    } else if (!ec) {
      auto transport = TcpTransportPtr(
          new TcpTransport(context, std::move(condat->socket), timeout, false));
      handler(api::kSuccess, transport);
    } else if (ec == boost::asio::error::operation_aborted) {
      handler(api::Error(YOGI_ERR_CANCELED), {});
    } else {
      handler(api::Error(YOGI_ERR_CONNECT_SOCKET_FAILED), {});
    }
  });

  condat->timer.expires_from_now(timeout);
  condat->timer.async_wait([=](auto& ec) {
    if (ec) return;

    condat->timed_out = true;
    CloseSocket(&condat->socket);
  });
}

void TcpTransport::WriteSome(boost::asio::const_buffer data,
                             CompletionHandler handler) {
  socket_.async_write_some(data, [=](auto& ec, auto bytes_written) {
    if (!ec) {
      handler(api::kSuccess, bytes_written);
    } else if (ec == boost::asio::error::operation_aborted) {
      handler(api::Error(YOGI_ERR_CANCELED), bytes_written);
    } else {
      handler(api::Error(YOGI_ERR_RW_SOCKET_FAILED), bytes_written);
    }
  });
}

void TcpTransport::ReadSome(boost::asio::mutable_buffer data,
                            CompletionHandler handler) {
  socket_.async_read_some(data, [=](auto& ec, auto bytes_read) {
    if (!ec) {
      handler(api::kSuccess, bytes_read);
    } else if (ec == boost::asio::error::operation_aborted) {
      handler(api::Error(YOGI_ERR_CANCELED), bytes_read);
    } else {
      handler(api::Error(YOGI_ERR_RW_SOCKET_FAILED), bytes_read);
    }
  });
}

void TcpTransport::Shutdown() { CloseSocket(&socket_); }

std::string TcpTransport::MakePeerDescription(
    const boost::asio::ip::tcp::socket& socket) {
  auto ep = socket.remote_endpoint();
  return MakeIpAddressString(ep) + ':' + std::to_string(ep.port());
}

void TcpTransport::CloseSocket(boost::asio::ip::tcp::socket* s) {
  boost::system::error_code ec;
  s->cancel(ec);
  s->shutdown(s->shutdown_both, ec);
  s->close(ec);
}

TcpTransport::TcpTransport(objects::ContextPtr context,
                           boost::asio::ip::tcp::socket&& socket,
                           std::chrono::nanoseconds timeout,
                           bool created_via_accept)
    : Transport(context, timeout, MakePeerDescription(socket)),
      socket_(std::move(socket)),
      created_via_accept_(created_via_accept) {}

void TcpTransport::SetNoDelayOption() {
  boost::system::error_code ec;
  socket_.set_option(boost::asio::ip::tcp::no_delay(true), ec);
  if (ec) {
    YOGI_LOG_WARNING(logger_, "Could not set TCP_NODELAY option on socket: "
                                  << ec.message());
  }
}

const objects::LoggerPtr TcpTransport::logger_ =
    objects::Logger::CreateStaticInternalLogger("Transport.Tcp");

}  // namespace network
