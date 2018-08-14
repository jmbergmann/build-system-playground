#pragma once

#include "object.h"
#include "internal/conversion.h"

#include <chrono>
#include <functional>
#include <memory>

namespace yogi {

YOGI_DEFINE_API_FN(int, YOGI_ContextCreate, (void** context))
YOGI_DEFINE_API_FN(int, YOGI_ContextPoll, (void* context, int* count))
YOGI_DEFINE_API_FN(int, YOGI_ContextPollOne, (void* context, int* count))
YOGI_DEFINE_API_FN(int, YOGI_ContextRun,
                   (void* context, int* count, long long duration))
YOGI_DEFINE_API_FN(int, YOGI_ContextRunOne,
                   (void* context, int* count, long long duration))
YOGI_DEFINE_API_FN(int, YOGI_ContextRunInBackground, (void* context))
YOGI_DEFINE_API_FN(int, YOGI_ContextStop, (void* context))
YOGI_DEFINE_API_FN(int, YOGI_ContextWaitForRunning,
                   (void* context, long long duration))
YOGI_DEFINE_API_FN(int, YOGI_ContextWaitForStopped,
                   (void* context, long long duration))
YOGI_DEFINE_API_FN(int, YOGI_ContextPost,
                   (void* context, void (*fn)(void* userarg), void* userarg))

/// Scheduler for the execution of asynchronous operations.
///
/// Once an asynchronous operation finishes, the corresponding handler function
/// is added to the context's event queue and executed through once of the
/// Poll... or Run... functions.
class Context : public Object {
 public:
  typedef std::function<void()> HandlerFn;

  /// Constructs the context.
  Context() : Object(internal::CallApiCreate(internal::YOGI_ContextCreate)) {}

  virtual const std::string& GetObjectTypeName() const {
    static std::string s = "Context";
    return s;
  }

  /// Runs the context's event processing loop to execute ready handlers.
  ///
  /// This function runs handlers (internal and user-supplied such as functions
  /// registered through Post()) that are ready to run, without blocking, until
  /// the Stop() function has been called or there are no more ready handlers.
  ///
  /// This function must be called from outside any handler functions that are
  /// being executed through the context.
  ///
  /// \returns Number of executed handlers.
  int Poll() {
    int count;
    int res = internal::YOGI_ContextPoll(GetHandle(), &count);
    internal::CheckErrorCode(res);
    return count;
  }

  /// Runs the context's event processing loop to execute at most one ready
  /// handler.
  ///
  /// This function runs at most one handler (internal and user-supplied such as
  /// functions registered through Post()) that are ready to run, without
  /// blocking.
  ///
  /// This function must be called from outside any handler functions that are
  /// being executed through the context.
  ///
  /// \returns Number of executed handlers.
  int PollOne() {
    int count;
    int res = internal::YOGI_ContextPollOne(GetHandle(), &count);
    internal::CheckErrorCode(res);
    return count;
  }

  /// Runs the context's event processing loop for the specified duration.
  ///
  /// This function blocks while running the context's event processing loop and
  /// calling dispatched handlers (internal and user-supplied such as functions
  /// registered through Post()) for the specified duration unless Stop() is
  /// called within that time.
  ///
  /// This function must be called from outside any handler functions that are
  /// being executed through the context.
  ///
  /// \param duration Duration.
  ///
  /// \returns Number of executed handlers.
  int Run(std::chrono::nanoseconds duration) {
    int count;
    int res = internal::YOGI_ContextRun(
        GetHandle(), &count, internal::DurationToCoreDuration(duration));
    internal::CheckErrorCode(res);
    return count;
  }

  /// Runs the context's event processing loop.
  ///
  /// This function blocks while running the context's event processing loop and
  /// calling dispatched handlers (internal and user-supplied such as functions
  /// registered through Post()) until Stop() is called.
  ///
  /// This function must be called from outside any handler functions that are
  /// being executed through the context.
  ///
  /// \returns Number of executed handlers.
  int Run() { return Run((std::chrono::nanoseconds::max)()); }

  /// Runs the context's event processing loop for the specified duration to
  /// execute at most one handler.
  ///
  /// This function blocks while running the context's event processing loop and
  /// calling dispatched handlers (internal and user-supplied such as functions
  /// registered through Post()) for the specified duration until a single
  /// handler function has been executed, unless Stop() is called within that
  /// time.
  ///
  /// This function must be called from outside any handler functions that are
  /// being executed through the context.
  ///
  /// \param duration Duration.
  ///
  /// \returns Number of executed handlers.
  int RunOne(std::chrono::nanoseconds duration) {
    int count;
    int res = internal::YOGI_ContextRunOne(
        GetHandle(), &count, internal::DurationToCoreDuration(duration));
    internal::CheckErrorCode(res);
    return count;
  }

  /// Runs the context's event processing loop to execute at most one handler.
  ///
  /// This function blocks while running the context's event processing loop and
  /// calling dispatched handlers (internal and user-supplied such as functions
  /// registered through Post()) until a single handler function has been
  /// executed, unless Stop() is called.
  ///
  /// This function must be called from outside any handler functions that are
  /// being executed through the context.
  ///
  /// \returns Number of executed handlers.
  int RunOne() { return RunOne((std::chrono::nanoseconds::max)()); }

  /// Starts an internal thread for running the context's event processing loop.
  ///
  /// This function starts a threads that runs the context's event processing
  /// loop in the background. It relieves the user from having to start a thread
  /// and calling the appropriate Run... or Poll... functions themself. The
  /// thread can be stopped using Stop().
  ///
  /// This function must be called from outside any handler functions that are
  /// being executed through the context.
  void RunInBackground() {
    int res = internal::YOGI_ContextRunInBackground(GetHandle());
    internal::CheckErrorCode(res);
  }

  /// Stops the context's event processing loop.
  ///
  /// This function signals the context to stop running its event processing
  /// loop. This causes Run... functions to return as soon as possible and it
  /// terminates the thread started via RunInBackground().
  void Stop() {
    int res = internal::YOGI_ContextStop(GetHandle());
    internal::CheckErrorCode(res);
  }

  /// Blocks until the context's event processing loop is being run or until the
  /// specified timeout is reached.
  ///
  /// This function must be called from outside any handler functions that are
  /// being executed through the context.
  ///
  /// \param duration Maximum time to wait.
  ///
  /// \returns True if the context's event processing loop is running within the
  ///          specified duration and false otherwise.
  bool WaitForRunning(std::chrono::nanoseconds duration) {
    int res = internal::YOGI_ContextWaitForRunning(
        GetHandle(), internal::DurationToCoreDuration(duration));
    if (res == static_cast<int>(ErrorCode::kTimeout)) return false;
    internal::CheckErrorCode(res);
    return true;
  }

  /// Blocks until the context's event processing loop is being run.
  ///
  /// This function must be called from outside any handler functions that are
  /// being executed through the context.
  void WaitForRunning() { WaitForRunning((std::chrono::nanoseconds::max)()); }

  /// Blocks until no thread is running the context's event processing
  /// loop or until the specified timeout is reached.
  ///
  /// This function must be called from outside any handler functions that are
  /// being executed through the context.
  ///
  /// \param duration Maximum time to wait.
  ///
  /// \returns True if the context's event processing loop is not running within
  ///          the specified duration and false otherwise.
  bool WaitForStopped(std::chrono::nanoseconds duration) {
    int res = internal::YOGI_ContextWaitForStopped(
        GetHandle(), internal::DurationToCoreDuration(duration));
    if (res == static_cast<int>(ErrorCode::kTimeout)) return false;
    internal::CheckErrorCode(res);
    return true;
  }

  /// Blocks until no thread is running the context's event processing loop.
  ///
  /// This function must be called from outside any handler functions that are
  /// being executed through the context.
  void WaitForStopped() { WaitForStopped((std::chrono::nanoseconds::max)()); }

  /// Adds the given function to the context's event processing queue to
  /// be executed and returns immediately.
  ///
  /// The handler \p fn will only be executed after this function returns and
  /// only by a thread running the context's event processing loop.
  ///
  /// \param fn Function to call from within the context.
  void Post(HandlerFn fn) {
    auto fn_ptr = new HandlerFn(fn);
    static auto wrapper = [](void* userarg) {
      std::unique_ptr<HandlerFn> fn_ptr(static_cast<HandlerFn*>(userarg));
      (*fn_ptr)();
    };

    int res = internal::YOGI_ContextPost(GetHandle(), wrapper, fn_ptr);
    internal::CheckErrorCode(res);
  }
};

}  // namespace yogi