#pragma once

#include "../config.h"
#include "../api/object.h"
#include "detail/log/console_sink.h"
#include "detail/log/hook_sink.h"
#include "detail/log/file_sink.h"

#include <atomic>
#include <mutex>
#include <vector>
#include <sstream>

#define YOGI_LOG_FATAL(logger, ...) YOGI_LOG(kFatal, logger, __VA_ARGS__)
#define YOGI_LOG_ERROR(logger, ...) YOGI_LOG(kError, logger, __VA_ARGS__)
#define YOGI_LOG_WARNING(logger, ...) YOGI_LOG(kWarning, logger, __VA_ARGS__)
#define YOGI_LOG_INFO(logger, ...) YOGI_LOG(kInfo, logger, __VA_ARGS__)
#define YOGI_LOG_DEBUG(logger, ...) YOGI_LOG(kDebug, logger, __VA_ARGS__)
#define YOGI_LOG_TRACE(logger, ...) YOGI_LOG(kTrace, logger, __VA_ARGS__)

#define YOGI_LOG(severity, logger, stream)                                    \
  {                                                                           \
    if (::objects::Logger::Verbosity::severity <= (logger)->GetVerbosity()) { \
      std::stringstream ss;                                                   \
      ss << stream;                                                           \
      (logger)->Log(::objects::Logger::Verbosity::severity, __FILE__,         \
                    __LINE__, ss.str().c_str());                              \
    }                                                                         \
  }

namespace objects {

class Logger : public api::ExposedObjectT<Logger, api::ObjectType::kLogger> {
 public:
  typedef detail::log::Sink::Verbosity Verbosity;

  static void SetSink(detail::log::ConsoleSinkPtr&& sink);
  static void SetSink(detail::log::HookSinkPtr&& sink);
  static void SetSink(detail::log::FileSinkPtr&& sink);
  static std::shared_ptr<Logger> GetAppLogger() { return app_logger_; }
  static std::shared_ptr<Logger> CreateInternalLogger(
      const std::string& component);
  static std::vector<std::weak_ptr<Logger>> GetInternalLoggers();

  Logger(std::string component);
  virtual ~Logger();

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

  static std::unique_lock<std::mutex> MakeInternalLoggersMutex();
  static std::vector<std::weak_ptr<Logger>>& InternalLoggers();

  const std::string component_;
  std::atomic<Verbosity> verbosity_;
};

typedef std::shared_ptr<Logger> LoggerPtr;

}  // namespace objects
