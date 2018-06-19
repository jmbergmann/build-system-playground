#include "macros.h"
#include "helpers.h"
#include "../objects/signal_set.h"

YOGI_API int YOGI_RaiseSignal(int signal, void* sigarg,
                              void (*fn)(void* sigarg, void* userarg),
                              void* userarg) {
  using Signals = objects::SignalSet::Signals;

  CHECK_PARAM(signal, Signals::kAllSignals);
  CHECK_PARAM(IsExactlyOneBitSet(signal));

  try {
    objects::SignalSet::CleanupHandler handler;
    if (fn) {
      handler = [fn, sigarg, userarg] { fn(sigarg, userarg); };
    } else {
      handler = [] {};
    }

    objects::SignalSet::RaiseSignal(ConvertFlags(signal, Signals::kNoSignal),
                                    sigarg, handler);
  }
  CATCH_AND_RETURN;
}

YOGI_API int YOGI_SignalSetCreate(void** sigset, void* context, int signals) {
  using Signals = objects::SignalSet::Signals;

  CHECK_PARAM(sigset != nullptr);
  CHECK_PARAM(context != nullptr);
  CHECK_FLAGS(signals, Signals::kAllSignals);

  try {
    auto ctx = api::ObjectRegister::Get<objects::Context>(context);
    auto set = objects::SignalSet::Create(
        ctx, ConvertFlags(signals, Signals::kNoSignal));
    *sigset = api::ObjectRegister::Register(set);
  }
  CATCH_AND_RETURN;
}

YOGI_API int YOGI_SignalSetAwait(void* sigset,
                                 void (*fn)(int res, int sig, void* sigarg,
                                            void* userarg),
                                 void* userarg) {
  CHECK_PARAM(sigset != nullptr);
  CHECK_PARAM(fn != nullptr);

  try {
    auto set = api::ObjectRegister::Get<objects::SignalSet>(sigset);
    set->Await([=](auto& res, auto signal, void* sigarg) {
      fn(res.error_code(), signal, sigarg, userarg);
    });
  }
  CATCH_AND_RETURN;
}

YOGI_API int YOGI_SignalSetCancelAwait(void* sigset) {
  CHECK_PARAM(sigset != nullptr);

  try {
    auto set = api::ObjectRegister::Get<objects::SignalSet>(sigset);
    set->CancelAwait();
  }
  CATCH_AND_RETURN;
}
