#include "timer.h"
#include "../api/error.h"

namespace objects {

Timer::Timer(ContextPtr context)
    : context_(context), timer_(context->IoContext()) {}

void Timer::Start(std::chrono::nanoseconds timeout,
                  std::function<void(const api::Error& res)> fn) {
  timer_.expires_after(timeout);
  timer_.async_wait([=](const auto& ec) {
    YOGI_ASSERT(!ec || ec == boost::asio::error::operation_aborted);
    fn(ec ? api::Error(YOGI_ERR_CANCELED) : api::kSuccess);
  });
}

bool Timer::Cancel() {
  return timer_.cancel() != 0;
}

}  // namespace objects
