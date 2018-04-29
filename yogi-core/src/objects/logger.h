#pragma once

#include "../config.h"
#include "../api/object.h"
#include "detail/log_sinks.h"

#include <boost/date_time/posix_time/ptime.hpp>

namespace objects {

class Logger : public api::ExposedObjectT<Logger, api::ObjectType::kLogger> {
 public:
  typedef detail::LogSink::Verbosity Verbosity;

  static void SetSink(detail::ConsoleLogSinkPtr&& sink) {
    console_sink_ = std::move(sink);
  }

  static void SetSink(detail::HookLogSinkPtr&& sink) {
    hook_sink_ = std::move(sink);
  }

  static void SetSink(detail::FileLogSinkPtr&& sink) {
    file_sink_ = std::move(sink);
  }

  static std::shared_ptr<Logger> GetAppLogger() { return app_logger_; }

  Logger(std::string component);

  const std::string& GetComponent() const { return component_; }
  void SetVerbosity(Verbosity verbosity) { verbosity_ = verbosity; }
  void Log(Verbosity severity, const char* file, int line, const char* msg);

 private:
  static detail::ConsoleLogSinkPtr console_sink_;
  static detail::HookLogSinkPtr hook_sink_;
  static detail::FileLogSinkPtr file_sink_;
  static std::shared_ptr<Logger> app_logger_;

  const std::string component_;
  Verbosity verbosity_;
};

typedef std::shared_ptr<Logger> LoggerPtr;

}  // namespace objects
