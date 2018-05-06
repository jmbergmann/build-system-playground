#include "hook_log_sink.h"

namespace objects {
namespace detail {

HookLogSink::HookLogSink(Verbosity verbosity, HookFn fn)
    : LogSink(verbosity), hook_fn_(fn) {}

void HookLogSink::WriteEntry(Verbosity severity,
                             const utils::Timestamp& timestamp, int tid,
                             const char* file, int line,
                             const std::string& component, const char* msg) {
  hook_fn_(severity, timestamp, tid, file, line, component, msg);
}

}  // namespace detail
}  // namespace objects
