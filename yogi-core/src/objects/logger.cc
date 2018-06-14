#include "logger.h"
#include "../api/constants.h"
#include "../utils/system.h"

#include <boost/algorithm/string.hpp>
#include <algorithm>
#include <stdexcept>

using namespace std::string_literals;

namespace objects {

Logger::Verbosity Logger::StringToVerbosity(const std::string& str) {
  if (boost::iequals(str, "NONE")) return Verbosity::kNone;
  if (boost::iequals(str, "FATAL")) return Verbosity::kFatal;
  if (boost::iequals(str, "ERROR")) return Verbosity::kError;
  if (boost::iequals(str, "WARNING")) return Verbosity::kWarning;
  if (boost::iequals(str, "INFO")) return Verbosity::kInfo;
  if (boost::iequals(str, "DEBUG")) return Verbosity::kDebug;
  if (boost::iequals(str, "TRACE")) return Verbosity::kTrace;

  throw std::runtime_error("Invalid verbosity \""s + str + "\"");
}

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

std::mutex Logger::sinks_mutex_;
detail::LogSinkPtr Logger::console_sink_;
detail::LogSinkPtr Logger::hook_sink_;
detail::LogSinkPtr Logger::file_sink_;
LoggerPtr Logger::app_logger_ = Logger::Create("App");

}  // namespace objects
