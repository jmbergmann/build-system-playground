#pragma once

#include "../config.h"
#include "context.h"

#include <boost/asio/high_resolution_timer.hpp>
#include <chrono>
#include <functional>
#include <mutex>
#include <condition_variable>

namespace objects {

class Timer : public api::ExposedObjectT<Timer, api::ObjectType::kTimer> {
 public:
  Timer(ContextPtr context);

  void Start(std::chrono::nanoseconds timeout, std::function<void (int res)> fn);
  bool Cancel();

 private:
  const ContextPtr context_;
  boost::asio::high_resolution_timer timer_;
};

typedef std::shared_ptr<Timer> TimerPtr;

}  // namespace objects
