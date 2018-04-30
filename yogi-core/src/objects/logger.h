#pragma once

#include "../config.h"
#include "../api/object.h"
#include "detail/log_sinks.h"

#include <atomic>
#include <mutex>

namespace objects {

class Logger : public api::ExposedObjectT<Logger, api::ObjectType::kLogger> {
 public:
  typedef detail::LogSink::Verbosity Verbosity;

  static void SetSink(detail::ConsoleLogSinkPtr&& sink);
  static void SetSink(detail::HookLogSinkPtr&& sink);
  static void SetSink(detail::FileLogSinkPtr&& sink);
  static std::shared_ptr<Logger> GetAppLogger() { return app_logger_; }

  Logger(std::string component);

  const std::string& GetComponent() const { return component_; }
  void SetVerbosity(Verbosity verbosity) { verbosity_ = verbosity; }
  void Log(Verbosity severity, const char* file, int line, const char* msg);

 private:
  static std::mutex sinks_mutex_;
  static detail::LogSinkPtr console_sink_;
  static detail::LogSinkPtr hook_sink_;
  static detail::LogSinkPtr file_sink_;
  static std::shared_ptr<Logger> app_logger_;

  const std::string component_;
  std::atomic<Verbosity> verbosity_;
};

typedef std::shared_ptr<Logger> LoggerPtr;

}  // namespace objects
