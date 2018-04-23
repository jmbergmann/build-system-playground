#pragma once

#include "../config.h"
#include "../api/object.h"

#include <boost/asio/io_service.hpp>
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

  boost::asio::io_service& IoService() { return ios_; }

  int Poll();
  int PollOne();
  int Run();
  int RunOne();
  int RunFor(std::chrono::nanoseconds dur);
  int RunOneFor(std::chrono::nanoseconds dur);
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

  boost::asio::io_service ios_;
  boost::asio::io_service::work work_;
  bool running_;
  std::mutex mutex_;
  std::condition_variable cv_;
  std::thread thread_;
};

typedef std::shared_ptr<Context> ContextPtr;

}  // namespace objects
