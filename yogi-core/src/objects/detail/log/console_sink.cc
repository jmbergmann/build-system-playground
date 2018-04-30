#include "console_sink.h"
#include "../../../utils/console.h"

#include <stdio.h>

namespace objects {
namespace detail {
namespace log {

ConsoleSink::ConsoleSink(Verbosity verbosity, FILE* stream, bool colour,
                         std::string time_fmt, std::string fmt)
    : TextBasedSink(verbosity, time_fmt, fmt, !colour),
      stream_(stream),
      colour_(colour) {}

void ConsoleSink::WritePartialOutput(const std::string& str) {
  fputs(str.c_str(), stream_);
}

void ConsoleSink::SetOutputColours(Verbosity severity) {
  switch (severity) {
    case Verbosity::kFatal:
      utils::SetConsoleColour(stream_, utils::ForegroundColour::kWhite);
      utils::SetConsoleColour(stream_, utils::BackgroundColour::kRed);
      break;

    case Verbosity::kError:
      utils::SetConsoleColour(stream_, utils::ForegroundColour::kLightRed);
      break;

    case Verbosity::kWarning:
      utils::SetConsoleColour(stream_, utils::ForegroundColour::kYellow);
      break;

    case Verbosity::kInfo:
      utils::SetConsoleColour(stream_, utils::ForegroundColour::kWhite);
      break;

    case Verbosity::kDebug:
      utils::SetConsoleColour(stream_, utils::ForegroundColour::kLightGreen);
      break;

    case Verbosity::kTrace:
      utils::SetConsoleColour(stream_, utils::ForegroundColour::kBrown);
      break;
  }
}

void ConsoleSink::ResetOutputColours() {
  utils::ResetConsoleColours(stream_);
}

void ConsoleSink::Flush() {
  fflush(stream_);
}

}  // namespace log
}  // namespace detail
}  // namespace objects
