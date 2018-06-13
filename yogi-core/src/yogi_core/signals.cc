#include "macros.h"
#include "helpers.h"
#include "../objects/context.h"

YOGI_API int YOGI_AwaitSignal(void* context, int signals,
                              void (*fn)(int res, int sig, void* userarg),
                              void* userarg) {
  using Signals = objects::Context::Signals;

  CHECK_PARAM(context != nullptr);
  CHECK_PARAM(fn != nullptr);
  CHECK_FLAGS(signals, Signals::kAllSignals);

  try {
    auto ctx = api::ObjectRegister::Get<objects::Context>(context);
    ctx->AwaitSignal(
        ConvertFlags(signals, Signals::kAllSignals),
        [=](auto& res, auto sig) { fn(res.error_code(), sig, userarg); });
  }
  CATCH_AND_RETURN;
}

YOGI_API int YOGI_CancelAwaitSignal(void* context) {
  CHECK_PARAM(context != nullptr);

  try {
    auto ctx = api::ObjectRegister::Get<objects::Context>(context);
    ctx->CancelAwaitSignal();
  }
  CATCH_AND_RETURN;
}
