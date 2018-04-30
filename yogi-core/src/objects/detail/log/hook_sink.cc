#include "hook_sink.h"

namespace objects {
namespace detail {
namespace log {

HookSink::HookSink(Verbosity verbosity, HookFn fn)
    : Sink(verbosity), hook_fn_(fn) {}

void HookSink::WriteEntry(Verbosity severity, const utils::Timestamp& timestamp,
                          int tid, const char* file, int line,
                          const std::string& component, const char* msg) {
  hook_fn_(severity, timestamp, tid, file, line, component, msg);
}

}  // namespace log
}  // namespace detail
}  // namespace objects
