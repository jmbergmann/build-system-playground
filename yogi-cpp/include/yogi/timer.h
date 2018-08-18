#pragma once

#include "duration.h"
#include "object.h"
#include "context.h"

#include <memory>
#include <functional>

namespace yogi {

YOGI_DEFINE_API_FN(int, YOGI_TimerCreate, (void** timer, void* context))
YOGI_DEFINE_API_FN(int, YOGI_TimerStart,
                   (void* timer, long long duration,
                    void (*fn)(int res, void* userarg), void* userarg))
YOGI_DEFINE_API_FN(int, YOGI_TimerCancel, (void* timer))

class Timer;
using TimerPtr = std::shared_ptr<Timer>;

class Timer : public ObjectT<Timer> {
 public:
  using HandlerFn = std::function<void(const Result& res)>;

  /// Creates a timer.
  ///
  /// \param context The context to use.
  ///
  /// \returns The created Timer.
  static TimerPtr Create(ContextPtr context) {
    return TimerPtr(new Timer(context));
  }

  /// Starts the timer.
  ///
  /// If the timer is already running, the timer will be canceled first,
  /// as if Cancel() were called explicitly.
  ///
  /// \param duration Time when the timer expires.
  /// \param fn       Handler function to call after the given time passed.
  void Start(const Duration& duration, HandlerFn fn) {
    struct CallbackData {
      HandlerFn fn;
    };

    auto data = std::make_unique<CallbackData>();
    data->fn = fn;

    int res = internal::YOGI_TimerStart(
        GetHandle(), internal::ToCoreDuration(duration),
        [](int res, void* userarg) {
          auto data = std::unique_ptr<CallbackData>(
              static_cast<CallbackData*>(userarg));

          internal::WithErrorCodeToResult(res, [&](const auto& result) {
            data->fn(result);
          });
        },
        data.get());

    internal::CheckErrorCode(res);
    data.release();
  }

  /// Cancels the timer.
  ///
  /// Canceling the timer will result in the handler function registered via
  /// Start() to be called with a cancellation error.static Note that if the
  /// handler is already scheduled for execution, it will be called without an
  /// error.
  ///
  /// \returns True if the timer was canceled successfully and false otherwise
  ///          (i.e. the timer has not been started or it already expired).
  bool Cancel() {
    int res = internal::YOGI_TimerCancel(GetHandle());
    if (res == static_cast<int>(ErrorCode::kTimerExpired)) {
      return false;
    }

    internal::CheckErrorCode(res);
    return true;
  }

 private:
  Timer(ContextPtr context)
      : ObjectT(internal::CallApiCreate(internal::YOGI_TimerCreate,
                                        GetForeignHandle(context)),
                {context}) {}
};

}  // namespace yogi
