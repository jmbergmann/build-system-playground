#include "logger.h"
#include "../api/constants.h"
#include "../utils/system.h"

#include <algorithm>

namespace objects {

std::mutex Logger::sinks_mutex_;
detail::log::SinkPtr Logger::console_sink_;
detail::log::SinkPtr Logger::hook_sink_;
detail::log::SinkPtr Logger::file_sink_;
std::mutex Logger::internal_loggers_mutex_;
std::vector<std::weak_ptr<Logger>> Logger::internal_loggers_;
LoggerPtr Logger::app_logger_ = Logger::Create("App");

void Logger::SetSink(detail::log::ConsoleSinkPtr&& sink) {
  std::lock_guard<std::mutex> lock(sinks_mutex_);
  console_sink_ = std::move(sink);
}

void Logger::SetSink(detail::log::HookSinkPtr&& sink) {
  std::lock_guard<std::mutex> lock(sinks_mutex_);
  hook_sink_ = std::move(sink);
}

void Logger::SetSink(detail::log::FileSinkPtr&& sink) {
  std::lock_guard<std::mutex> lock(sinks_mutex_);
  file_sink_ = std::move(sink);
}

LoggerPtr Logger::CreateInternalLogger(const std::string& component) {
  auto logger = std::make_shared<Logger>(std::string("Yogi.") + component);
  std::lock_guard<std::mutex> lock(internal_loggers_mutex_);
  internal_loggers_.push_back(logger);
  return logger;
}

std::vector<std::weak_ptr<Logger>> Logger::GetInternalLoggers() {
  std::lock_guard<std::mutex> lock(internal_loggers_mutex_);
  return internal_loggers_;
}

Logger::Logger(std::string component)
    : component_(component),
      verbosity_(static_cast<Verbosity>(api::kDefaultLoggerVerbosity)) {}

Logger::~Logger() {
  std::lock_guard<std::mutex> lock(internal_loggers_mutex_);
  internal_loggers_.erase(
      std::remove_if(internal_loggers_.begin(), internal_loggers_.end(),
                     [](auto& ptr) { return ptr.expired(); }),
      internal_loggers_.end());
}

void Logger::Log(Verbosity severity, const char* file, int line,
                 const char* msg) {
  if (severity > verbosity_) {
    return;
  }

  auto timestamp = utils::Timestamp::Now();
  int tid = utils::GetCurrentThreadId();

  std::lock_guard<std::mutex> lock(sinks_mutex_);
  detail::log::SinkPtr* sinks[] = {&console_sink_, &hook_sink_, &file_sink_};
  for (auto sink : sinks) {
    if (*sink) {
      (*sink)->Publish(severity, timestamp, tid, file, line, component_, msg);
    }
  }
}

}  // namespace objects
