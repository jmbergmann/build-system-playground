#include "signal_set.h"

#include <algorithm>

namespace objects {

void SignalSet::RaiseSignal(Signals signal, void* sigarg,
                            CleanupHandler cleanup_handler) {
  YOGI_ASSERT(signal != kNoSignal);

  auto data = std::make_shared<SignalData>();
  data->signal = signal;
  data->sigarg = sigarg;
  data->cleanup_handler = cleanup_handler;

  auto sets = api::ObjectRegister::GetMatching<SignalSet>(
      [&](auto& set) { return set->GetSignals() & signal; });

  data->cnt = static_cast<int>(sets.size());
  if (data->cnt == 0) {
    data->cleanup_handler();
    return;
  }

  for (auto& set : sets) {
    set->OnSignalRaised(data);
  }
}

SignalSet::SignalSet(ContextPtr context, Signals signals)
    : context_(context), signals_(signals) {}

void SignalSet::Await(AwaitHandler handler) {
  std::lock_guard<std::mutex> lock(mutex_);

  if (handler_) {
    auto old_handler = handler_;
    context_->Post([old_handler] {
      old_handler(api::Error(YOGI_ERR_CANCELED), kNoSignal, nullptr);
    });
  }

  handler_ = handler;

  if (!queue_.empty()) {
    DeliverNextSignal();
  }
}

SignalSet::~SignalSet() {
  while (!queue_.empty()) {
    auto data = queue_.front();
    queue_.pop();

    --data->cnt;
    if (data->cnt == 0) {
      context_->Post([data] { data->cleanup_handler(); });
    }
  }

  CancelAwait();
}

void SignalSet::CancelAwait() { Await({}); }

void SignalSet::OnSignalRaised(const SignalDataPtr& data) {
  YOGI_ASSERT(signals_ & data->signal);

  std::lock_guard<std::mutex> lock(mutex_);
  queue_.push(data);
  DeliverNextSignal();
}

void SignalSet::DeliverNextSignal() {
  YOGI_ASSERT(!queue_.empty());

  if (!handler_) {
    return;
  }

  AwaitHandler handler;
  std::swap(handler_, handler);

  auto data = queue_.front();
  queue_.pop();

  context_->Post([handler, data] {
    handler(api::kSuccess, data->signal, data->sigarg);
    --data->cnt;
    if (data->cnt == 0) {
      data->cleanup_handler();
    }
  });
}

}  // namespace objects
