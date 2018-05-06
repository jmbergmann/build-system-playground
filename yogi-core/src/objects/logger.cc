#include "logger.h"
#include "../api/constants.h"
#include "../utils/system.h"

#include <algorithm>

namespace objects {

std::mutex Logger::sinks_mutex_;
detail::LogSinkPtr Logger::console_sink_;
detail::LogSinkPtr Logger::hook_sink_;
detail::LogSinkPtr Logger::file_sink_;
LoggerPtr Logger::app_logger_ = Logger::Create("App");

void Logger::SetSink(detail::ConsoleLogSinkPtr&& sink) {
  std::lock_guard<std::mutex> lock(sinks_mutex_);
  console_sink_ = std::move(sink);
}

void Logger::SetSink(detail::HookLogSinkPtr&& sink) {
  std::lock_guard<std::mutex> lock(sinks_mutex_);
  hook_sink_ = std::move(sink);
}

void Logger::SetSink(detail::FileLogSinkPtr&& sink) {
  std::lock_guard<std::mutex> lock(sinks_mutex_);
  file_sink_ = std::move(sink);
}

LoggerPtr Logger::CreateStaticInternalLogger(const std::string& component) {
  auto logger = std::make_shared<Logger>(std::string("Yogi.") + component);
  InternalLoggers().push_back(logger);
  return logger;
}

Logger::Logger(std::string component)
    : component_(component),
      verbosity_(static_cast<Verbosity>(api::kDefaultLoggerVerbosity)) {}

void Logger::Log(Verbosity severity, const char* file, int line,
                 const char* msg) {
  if (severity > verbosity_) {
    return;
  }

  auto timestamp = utils::Timestamp::Now();
  int tid = utils::GetCurrentThreadId();

  std::lock_guard<std::mutex> lock(sinks_mutex_);
  detail::LogSinkPtr* sinks[] = {&console_sink_, &hook_sink_, &file_sink_};
  for (auto sink : sinks) {
    if (*sink) {
      (*sink)->Publish(severity, timestamp, tid, file, line, component_, msg);
    }
  }
}

std::vector<std::weak_ptr<Logger>>& Logger::InternalLoggers() {
  static std::vector<std::weak_ptr<Logger>> vec;
  return vec;
}

}  // namespace objects
