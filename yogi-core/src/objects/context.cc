#include "context.h"
#include "../api/error.h"

#include <boost/asio/post.hpp>

namespace objects {

const LoggerPtr Context::logger_ =
    Logger::CreateStaticInternalLogger("Context");

Context::Context() : work_(ioc_), running_(false) {
}

Context::~Context() {
  Stop();
  WaitForStopped(std::chrono::nanoseconds::max());
}

int Context::Poll() {
  return RunImpl([&] { return ioc_.poll(); });
}

int Context::PollOne() {
  return RunImpl([&] { return ioc_.poll_one(); });
}

int Context::Run(std::chrono::nanoseconds dur) {
  return RunImpl([&] {
    if (dur == dur.max()) {
      return ioc_.run();
    } else {
      return ioc_.run_for(dur);
    }
  });
}

int Context::RunOne(std::chrono::nanoseconds dur) {
  return RunImpl([&] {
    if (dur == dur.max()) {
      return ioc_.run_one();
    } else {
      return ioc_.run_one_for(dur);
    }
  });
}

void Context::RunInBackground() {
  SetRunningFlagAndReset();
  thread_ = std::thread([&] {
    try {
      ioc_.run();
    }
    catch (const std::exception& e) {
      YOGI_LOG_FATAL(logger_, "Exception caught in context background thread: " << e.what());
    }
    catch (...) {
      YOGI_LOG_FATAL(logger_, "Unknown Exception caught in context background thread");
    }

    ClearRunningFlag();
  });
}

void Context::Stop() {
  std::lock_guard<std::mutex> lock(mutex_);
  ioc_.stop();
}

bool Context::WaitForRunning(std::chrono::nanoseconds timeout) {
  std::unique_lock<std::mutex> lock(mutex_);
  bool timed_out = false;
  if (timeout == timeout.max()) {
    cv_.wait(lock, [&] { return running_; });
  }
  else {
    timed_out = !cv_.wait_for(lock, timeout, [&] { return running_; });
  }

  return !timed_out;
}

bool Context::WaitForStopped(std::chrono::nanoseconds timeout) {
  std::unique_lock<std::mutex> lock(mutex_);
  bool timed_out = false;
  if (timeout == timeout.max()) {
    cv_.wait(lock, [&] { return !running_; });
  }
  else {
    timed_out = !cv_.wait_for(lock, timeout, [&] { return !running_; });
  }

  if (!timed_out && thread_.joinable()) {
    thread_.join();
  }

  return !timed_out;
}

void Context::Post(std::function<void ()> fn) {
  boost::asio::post(ioc_, fn);
}

void Context::SetRunningFlagAndReset() {
  std::lock_guard<std::mutex> lock(mutex_);
  if (running_) {
    throw api::Error(YOGI_ERR_BUSY);
  }

  running_ = true;
  ioc_.restart();
  cv_.notify_all();
}

void Context::ClearRunningFlag() {
  std::lock_guard<std::mutex> lock(mutex_);
  YOGI_ASSERT(running_ == true);
  running_ = false;
  cv_.notify_all();
}

template <typename Fn>
int Context::RunImpl(Fn fn) {
  SetRunningFlagAndReset();
  int cnt = static_cast<int>(fn());
  ClearRunningFlag();
  return cnt;
}

}  // namespace objects
