#include "log_sinks.h"

namespace objects {
namespace detail {

void LogSink::Put(Verbosity severity, const utils::Timestamp& timestamp,
                  int tid, const char* file, int line,
                  const std::string& component, const char* msg) {
  if (severity <= verbosity_) {
    Write(severity, timestamp, tid, file, line, component, msg);
  }
}

ConsoleLogSink::ConsoleLogSink(FILE* stream, bool colour, std::string timefmt,
                               std::string fmt, Verbosity verbosity)
    : LogSink(verbosity), stream_(stream), colour_(colour), fmt_(fmt) {}

void ConsoleLogSink::Write(Verbosity severity,
                           const utils::Timestamp& timestamp, int tid,
                           const char* file, int line,
                           const std::string& component, const char* msg) {}

HookLogSink::HookLogSink(HookFn fn, Verbosity verbosity)
    : LogSink(verbosity), hook_fn_(fn) {}

void HookLogSink::Write(Verbosity severity, const utils::Timestamp& timestamp,
                        int tid, const char* file, int line,
                        const std::string& component, const char* msg) {
  hook_fn_(severity, timestamp, tid, file, line, component, msg);
}

FileLogSink::FileLogSink(std::string filename, std::string timefmt,
                         std::string fmt, Verbosity verbosity)
    : LogSink(verbosity), filename_(filename), fmt_(fmt) {}

void FileLogSink::Write(Verbosity severity, const utils::Timestamp& timestamp,
                        int tid, const char* file, int line,
                        const std::string& component, const char* msg) {}

}  // namespace detail
}  // namespace objects
