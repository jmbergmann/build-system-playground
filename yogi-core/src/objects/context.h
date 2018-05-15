#pragma once

#include "../config.h"
#include "../api/object.h"
#include "logger.h"

#include <boost/asio/io_context.hpp>
#include <chrono>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <thread>

namespace objects {

class Context : public api::ExposedObjectT<Context, api::ObjectType::kContext> {
 public:
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
  void Post(std::function<void ()> fn);

 private:
  void SetRunningFlagAndReset();
  void ClearRunningFlag();

  template <typename Fn>
  int RunImpl(Fn fn);

  static const LoggerPtr logger_;

  boost::asio::io_context ioc_;
  boost::asio::io_context::work work_;
  bool running_;
  std::mutex mutex_;
  std::condition_variable cv_;
  std::thread thread_;
};

typedef std::shared_ptr<Context> ContextPtr;

}  // namespace objects
