#pragma once

#include "../config.h"
#include "../api/object.h"
#include "logger.h"

#include <boost/asio/io_context.hpp>
#include <boost/asio/signal_set.hpp>
#include <chrono>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <thread>

namespace objects {

class Context : public api::ExposedObjectT<Context, api::ObjectType::kContext> {
 public:
  enum Signals {
    kNoSignal = 0,
    kSigInt = YOGI_SIG_INT,
    kSigTerm = YOGI_SIG_TERM,
    kAllSignals = YOGI_SIG_INT | YOGI_SIG_TERM,
  };

  typedef std::function<void(const api::Error&, Signals)> SignalHandler;

  Context();
  virtual ~Context();

  boost::asio::io_context& IoContext() { return ioc_; }

  int Poll();
  int PollOne();
  int Run(std::chrono::nanoseconds dur);
  int RunOne(std::chrono::nanoseconds dur);
  void RunInBackground();
  void Stop();
  bool WaitForRunning(std::chrono::nanoseconds timeout);
  bool WaitForStopped(std::chrono::nanoseconds timeout);
  void Post(std::function<void()> fn);
  void AwaitSignal(Signals signals, SignalHandler signal_handler);
  void CancelAwaitSignal();

 private:
  void SetRunningFlagAndReset();
  void ClearRunningFlag();

  template <typename Fn>
  int RunImpl(Fn fn);

  static const LoggerPtr logger_;

  boost::asio::io_context ioc_;
  boost::asio::io_context::work work_;
  boost::asio::signal_set signals_;
  bool running_;
  std::mutex mutex_;
  std::condition_variable cv_;
  std::thread thread_;
};

typedef std::shared_ptr<Context> ContextPtr;

inline Context::Signals operator|(Context::Signals a, Context::Signals b) {
  return static_cast<Context::Signals>(static_cast<int>(a) |
                                       static_cast<int>(b));
}

inline Context::Signals operator&(Context::Signals a, Context::Signals b) {
  return static_cast<Context::Signals>(static_cast<int>(a) &
                                       static_cast<int>(b));
}

}  // namespace objects
