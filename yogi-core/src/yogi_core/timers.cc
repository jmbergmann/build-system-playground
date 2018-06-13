#include "macros.h"
#include "helpers.h"
#include "../objects/timer.h"

YOGI_API int YOGI_TimerCreate(void** timer, void* context) {
  CHECK_PARAM(timer != nullptr);
  CHECK_PARAM(context != nullptr);

  try {
    auto ctx = api::ObjectRegister::Get<objects::Context>(context);
    auto tmr = objects::Timer::Create(ctx);
    *timer = api::ObjectRegister::Register(tmr);
  }
  CATCH_AND_RETURN;
}

YOGI_API int YOGI_TimerStart(void* timer, long long duration,
                             void (*fn)(int, void*), void* userarg) {
  CHECK_PARAM(timer != nullptr);
  CHECK_PARAM(duration >= -1);
  CHECK_PARAM(fn != nullptr);

  try {
    auto tmr = api::ObjectRegister::Get<objects::Timer>(timer);
    auto timeout = ConvertDuration(duration);
    tmr->Start(timeout, [=](auto& res) { fn(res.error_code(), userarg); });
  }
  CATCH_AND_RETURN;
}

YOGI_API int YOGI_TimerCancel(void* timer) {
  CHECK_PARAM(timer != nullptr);

  try {
    auto tmr = api::ObjectRegister::Get<objects::Timer>(timer);
    if (!tmr->Cancel()) {
      return YOGI_ERR_TIMER_EXPIRED;
    }
  }
  CATCH_AND_RETURN;
}
