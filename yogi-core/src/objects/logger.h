#pragma once

#include "../config.h"
#include "../api/object.h"
#include "detail/log/console_sink.h"
#include "detail/log/hook_sink.h"
#include "detail/log/file_sink.h"

#include <atomic>
#include <mutex>

namespace objects {

class Logger : public api::ExposedObjectT<Logger, api::ObjectType::kLogger> {
 public:
  typedef detail::log::Sink::Verbosity Verbosity;

  static void SetSink(detail::log::ConsoleSinkPtr&& sink);
  static void SetSink(detail::log::HookSinkPtr&& sink);
  static void SetSink(detail::log::FileSinkPtr&& sink);
  static std::shared_ptr<Logger> GetAppLogger() { return app_logger_; }

  Logger(std::string component);

  const std::string& GetComponent() const { return component_; }
  Verbosity GetVerbosity() const { return verbosity_; }
  void SetVerbosity(Verbosity verbosity) { verbosity_ = verbosity; }
  void Log(Verbosity severity, const char* file, int line, const char* msg);

 private:
  static std::mutex sinks_mutex_;
  static detail::log::SinkPtr console_sink_;
  static detail::log::SinkPtr hook_sink_;
  static detail::log::SinkPtr file_sink_;
  static std::shared_ptr<Logger> app_logger_;

  const std::string component_;
  std::atomic<Verbosity> verbosity_;
};

typedef std::shared_ptr<Logger> LoggerPtr;

}  // namespace objects
