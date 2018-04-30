#include "text_based_sink.h"

namespace objects {
namespace detail {
namespace log {

TextBasedSink::TextBasedSink(Verbosity verbosity, std::string time_fmt,
                             std::string fmt)
    : Sink(verbosity), time_fmt_(time_fmt_), fmt_(fmt) {}

void TextBasedSink::WriteEntry(Verbosity severity,
                               const utils::Timestamp& timestamp, int tid,
                               const char* file, int line,
                               const std::string& component, const char* msg) {
}

}  // namespace log
}  // namespace detail
}  // namespace objects
