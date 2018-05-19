#include "socket.h"

namespace utils {

TimedTcpSocket::TimedTcpSocket(objects::ContextPtr context,
                               std::chrono::nanoseconds timeout)
    : timeout_(timeout),
      socket_(context->IoContext()),
      timer_(context->IoContext()),
      timed_out_(false),
      rcv_buffer_(std::make_shared<std::vector<char>>()) {}

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
