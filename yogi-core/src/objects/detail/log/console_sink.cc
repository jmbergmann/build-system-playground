#include "console_sink.h"

#include <stdio.h>

namespace objects {
namespace detail {
namespace log {

ConsoleSink::ConsoleSink(Verbosity verbosity, FILE* stream, bool colour,
                         std::string time_fmt, std::string fmt)
    : TextBasedSink(verbosity, time_fmt, fmt),
      stream_(stream),
      colour_(colour) {}

void ConsoleSink::WritePartialOutput(const std::string& str) {
  fputs(str.c_str(), stream_);
}

void ConsoleSink::SetOutputColour(bool on) {}

}  // namespace log
}  // namespace detail
}  // namespace objects
