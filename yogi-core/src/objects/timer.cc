#include "timer.h"
#include "../api/error.h"

namespace objects {

Timer::Timer(ContextPtr context)
    : context_(context), timer_(context->IoContext()) {}

void Timer::Start(std::chrono::nanoseconds timeout,
                  std::function<void(int res)> fn) {
  timer_.expires_from_now(timeout);
  timer_.async_wait([=](const auto& ec) {
    YOGI_ASSERT(!ec || ec == boost::asio::error::operation_aborted);
    fn(ec ? YOGI_ERR_CANCELED : YOGI_OK);
  });
}

bool Timer::Cancel() {
  return timer_.cancel() != 0;
}

}  // namespace objects
