#include "log_sinks.h"

namespace objects {
namespace detail {

ConsoleLogSink::ConsoleLogSink(FILE* stream, bool colour, std::string fmt,
                               Verbosity verbosity) {}

HookLogSink::HookLogSink(HookFn fn, Verbosity verbosity) {}

FileLogSink::FileLogSink(std::string filename, std::string fmt,
                         Verbosity verbosity) {}

}  // namespace detail
}  // namespace objects
