#include "logger.h"
#include "../api/constants.h"
#include "../utils/system.h"

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

Logger::Logger(std::string component)
    : component_(component),
      verbosity_(static_cast<Verbosity>(api::kDefaultLogVerbosity)) {}

void Logger::Log(Verbosity severity, const char* file, int line,
                 const char* msg) {
  auto timestamp = utils::Timestamp::Now();
  int tid = utils::GetCurrentThreadId();

  std::lock_guard<std::mutex> lock(sinks_mutex_);
  detail::LogSinkPtr* sinks[] = {&console_sink_, &hook_sink_, &file_sink_};
  for (auto sink : sinks) {
    if (*sink) {
      (*sink)->Put(severity, timestamp, tid, file, line, component_, msg);
    }
  }
}

}  // namespace objects
