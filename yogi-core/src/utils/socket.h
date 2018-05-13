#pragma once

#include "../config.h"
#include "../objects/context.h"
#include "../api/error.h"

#include <boost/asio.hpp>
#include <memory>
#include <chrono>

namespace utils {

class TimedTcpSocket : public std::enable_shared_from_this<TimedTcpSocket> {
 public:
  TimedTcpSocket(objects::ContextPtr context, std::chrono::nanoseconds timeout);

  boost::asio::ip::tcp::socket& Socket() { return socket_; }

  template <typename ConnectHandler>
  void Connect(const boost::asio::ip::tcp::endpoint& ep,
               ConnectHandler&& handler) {
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

  template <typename ConstBufferSequence, typename SendHandler>
  void Send(const ConstBufferSequence& buffers, SendHandler&& handler) {
    auto weak_self = std::weak_ptr<TimedTcpSocket>{shared_from_this()};
    boost::asio::async_write(socket_, buffers,
                             [weak_self, handler = std::move(handler)](
                                 auto& ec, auto bytes_written) {
                               auto self = weak_self.lock();
                               if (!self) return;

                               if (!ec) {
                                 handler(api::kSuccess);
                               } else {
                                 handler(api::Error(YOGI_ERR_RW_SOCKET_FAILED));
                               }
                             });
  }

  template <typename MutableBufferSequence, typename ReceiveHandler>
  void Receive(const MutableBufferSequence& buffers, ReceiveHandler&& handler) {
    auto weak_self = std::weak_ptr<TimedTcpSocket>{shared_from_this()};
    boost::asio::async_read(
        socket_, buffers,
        [weak_self, handler = std::move(handler)](auto& ec, auto bytes_read) {
          auto self = weak_self.lock();
          if (!self) return;

          self->timer_.cancel();

          if (self->timed_out_) {
            handler(api::Error(YOGI_ERR_TIMEOUT));
          } else if (!ec) {
            handler(api::kSuccess);
          } else {
            handler(api::Error(YOGI_ERR_RW_SOCKET_FAILED));
          }
        });

    StartTimeout(weak_self);
  }

 private:
  void StartTimeout(std::weak_ptr<TimedTcpSocket> weak_self);
  void OnTimeout();

  const std::chrono::nanoseconds timeout_;
  boost::asio::ip::tcp::socket socket_;
  boost::asio::steady_timer timer_;
  bool timed_out_;
};

typedef std::shared_ptr<TimedTcpSocket> TimedTcpSocketPtr;

}  // namespace utils
