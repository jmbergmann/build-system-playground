#include "console_log_sink.h"
#include "../../../utils/console.h"

#include <stdio.h>

namespace objects {
namespace detail {

ConsoleLogSink::ConsoleLogSink(Verbosity verbosity, FILE* stream, bool color,
                               std::string time_fmt, std::string fmt)
    : TextBasedLogSink(verbosity, time_fmt, fmt, !color),
      stream_(stream),
      colour_(color) {}

void ConsoleLogSink::WritePartialOutput(const std::string& str) {
  fputs(str.c_str(), stream_);
}

void ConsoleLogSink::SetOutputColours(Verbosity severity) {
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

    default:
      YOGI_NEVER_REACHED;
      break;
  }
}

void ConsoleLogSink::ResetOutputColours() {
  utils::ResetConsoleColours(stream_);
}

void ConsoleLogSink::Flush() { fflush(stream_); }

}  // namespace detail
}  // namespace objects
