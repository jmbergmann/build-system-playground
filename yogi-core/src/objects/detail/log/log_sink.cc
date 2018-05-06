#include "log_sink.h"

namespace objects {
namespace detail {

void LogSink::Publish(Verbosity severity, const utils::Timestamp& timestamp,
                      int tid, const char* file, int line,
                      const std::string& component, const char* msg) {
  if (severity <= verbosity_) {
    WriteEntry(severity, timestamp, tid, file, line, component, msg);
  }
}

}  // namespace detail
}  // namespace objects
