#include "logger.h"
#include "../api/constants.h"

namespace objects {

detail::ConsoleLogSinkPtr Logger::console_sink_;
detail::HookLogSinkPtr Logger::hook_sink_;
detail::FileLogSinkPtr Logger::file_sink_;
LoggerPtr Logger::app_logger_ = Logger::Create("App");

Logger::Logger(std::string component)
    : component_(component),
      verbosity_(static_cast<Verbosity>(api::kDefaultLogVerbosity)) {}

void Logger::Log(Verbosity severity, const char* file, int line,
                 const char* msg) {
  printf("%s\n", msg);  // TODO
}

}  // namespace objects
