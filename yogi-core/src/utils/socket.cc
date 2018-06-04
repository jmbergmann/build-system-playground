#include "socket.h"

namespace utils {

TimedTcpSocket::TimedTcpSocket(objects::ContextPtr context,
                               std::chrono::nanoseconds timeout)
    : context_(context),
      timeout_(timeout),
      rcv_buffer_(MakeSharedByteVector()),
      socket_(context->IoContext()),
      timer_(context->IoContext()),
      timed_out_(false) {}

void TimedTcpSocket::Accept(boost::asio::ip::tcp::acceptor* acceptor,
                            CompletionHandler handler) {
  remote_ep_ = {};
  auto weak_self = std::weak_ptr<TimedTcpSocket>{shared_from_this()};
  acceptor->async_accept(socket_,
                         [weak_self, handler = std::move(handler)](auto& ec) {
                           auto self = weak_self.lock();
                           if (!self) return;

                           if (!ec) {
                             self->remote_ep_ = self->socket_.remote_endpoint();
                             handler(api::kSuccess);
                           } else {
                             handler(api::Error(YOGI_ERR_ACCEPT_SOCKET_FAILED));
                           }
                         });
}

void TimedTcpSocket::Connect(const boost::asio::ip::tcp::endpoint& ep,
                             CompletionHandler handler) {
  remote_ep_ = ep;
  auto weak_self = std::weak_ptr<TimedTcpSocket>{shared_from_this()};
  socket_.async_connect(ep,
                        [weak_self, handler = std::move(handler)](auto& ec) {
                          auto self = weak_self.lock();
                          if (!self) return;

                          if (self->timed_out_) {
                            handler(api::Error(YOGI_ERR_TIMEOUT));
                          } else if (!ec) {
                            handler(api::kSuccess);
                          } else {
                            handler(api::Error(YOGI_ERR_CONNECT_SOCKET_FAILED));
                          }
                        });

  StartTimeout(weak_self);
}

void TimedTcpSocket::Send(SharedByteVector data, CompletionHandler handler) {
  auto weak_self = std::weak_ptr<TimedTcpSocket>{shared_from_this()};
  boost::asio::async_write(
      socket_, boost::asio::buffer(*data),
      [weak_self, data, handler = std::move(handler)](auto& ec, auto) {
        auto self = weak_self.lock();
        if (!self) return;

        if (!ec) {
          handler(api::kSuccess);
        } else {
          handler(api::Error(YOGI_ERR_RW_SOCKET_FAILED));
        }
      });
}

void TimedTcpSocket::ReceiveExactly(std::size_t num_bytes,
                                    ReceiveHandler handler) {
  auto buffer = rcv_buffer_;
  buffer->resize(num_bytes);
  auto weak_self = std::weak_ptr<TimedTcpSocket>{shared_from_this()};
  boost::asio::async_read(socket_, boost::asio::buffer(*buffer),
                          [weak_self, buffer, handler = std::move(handler)](
                              auto& ec, auto bytes_read) {
                            auto self = weak_self.lock();
                            if (!self) return;

                            self->timer_.cancel();

                            if (self->timed_out_) {
                              handler(api::Error(YOGI_ERR_TIMEOUT), *buffer);
                            } else if (!ec) {
                              handler(api::kSuccess, *buffer);
                            } else {
                              handler(api::Error(YOGI_ERR_RW_SOCKET_FAILED),
                                      *buffer);
                            }
                          });

  StartTimeout(weak_self);
}

void TimedTcpSocket::StartTimeout(std::weak_ptr<TimedTcpSocket> weak_self) {
  timer_.expires_from_now(timeout_);
  timer_.async_wait([weak_self](auto& ec) {
    auto self = weak_self.lock();
    if (!self) return;

    if (!ec) {
      self->OnTimeout();
    }
  });
}

void TimedTcpSocket::OnTimeout() {
  timed_out_ = true;

  boost::system::error_code ec;
  socket_.cancel(ec);
  socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
  socket_.close(ec);
}

}  // namespace utils

std::ostream& operator<<(std::ostream& os,
                         const utils::TimedTcpSocket& socket) {
  auto& ep = socket.GetRemoteEndpoint();
  os << ep.address().to_string() << ":" << ep.port();
  return os;
}
