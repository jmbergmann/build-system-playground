#include "log_sinks.h"

namespace objects {
namespace detail {

void LogSink::Put(Verbosity verbosity, const utils::Timestamp& timestamp,
                  int tid, const char* file, int line,
                  const std::string& component, const char* msg) {
  if (verbosity <= verbosity_) {
    Write(verbosity, timestamp, tid, file, line, component, msg);
  }
}

ConsoleLogSink::ConsoleLogSink(FILE* stream, bool colour, std::string fmt,
                               Verbosity verbosity)
    : LogSink(verbosity), stream_(stream), colour_(colour), fmt_(fmt) {}

void ConsoleLogSink::Write(Verbosity verbosity,
                           const utils::Timestamp& timestamp, int tid,
                           const char* file, int line,
                           const std::string& component, const char* msg) {}

HookLogSink::HookLogSink(HookFn fn, Verbosity verbosity)
    : LogSink(verbosity), hook_fn_(fn) {}

void HookLogSink::Write(Verbosity verbosity, const utils::Timestamp& timestamp,
                        int tid, const char* file, int line,
                        const std::string& component, const char* msg) {
  hook_fn_(verbosity, timestamp, tid, file, line, component, msg);
}

FileLogSink::FileLogSink(std::string filename, std::string fmt,
                         Verbosity verbosity)
    : LogSink(verbosity), filename_(filename), fmt_(fmt) {}

void FileLogSink::Write(Verbosity verbosity, const utils::Timestamp& timestamp,
                        int tid, const char* file, int line,
                        const std::string& component, const char* msg) {}

}  // namespace detail
}  // namespace objects
