#include "console.h"

#include "stdio.h"

#ifdef _WIN32
#   include <Windows.h>
#else
#   include <unistd.h>
#endif

namespace utils {
namespace {

#ifdef _WIN32
auto win32_original_stdout_colours = []() {
  HANDLE hdl = GetStdHandle(STD_OUTPUT_HANDLE);
  CONSOLE_SCREEN_BUFFER_INFO csbi;
  GetConsoleScreenBufferInfo(hdl, &csbi);
  return csbi.wAttributes;
}();

auto win32_original_stderr_colours = []() {
  HANDLE hdl = GetStdHandle(STD_ERROR_HANDLE);
  CONSOLE_SCREEN_BUFFER_INFO csbi;
  GetConsoleScreenBufferInfo(hdl, &csbi);
  return csbi.wAttributes;
}();
#endif

}  // anonymous namespace

void SetConsoleTitle(FILE* stream, const std::string& title) {
#ifdef _WIN32
    ::SetConsoleTitle(title.c_str());
#else
    if (isatty(stream == stdout ? STDOUT_FILENO : STDERR_FILENO)) {
      fprintf(stream, "\033]0;%s\007", title.c_str());
      fflush(stream);
    }
#endif
}

void SetConsoleColour(FILE* stream, ForegroundColour colour) {
#ifdef _WIN32
  HANDLE hdl =
      GetStdHandle(stream == stdout ? STD_OUTPUT_HANDLE : STD_ERROR_HANDLE);
  CONSOLE_SCREEN_BUFFER_INFO csbi;
  GetConsoleScreenBufferInfo(hdl, &csbi);
  SetConsoleTextAttribute(
      hdl, (csbi.wAttributes & 0xFFF0) | static_cast<WORD>(colour));
#else
    if (!isatty(stream == stdout ? STDOUT_FILENO : STDERR_FILENO)) {
        return;
    }

    const char* seq;
    switch (colour) {
      case ForegroundColour::kBlack:        seq = "\033[22;30m"; break;
      case ForegroundColour::kBlue:         seq = "\033[22;34m"; break;
      case ForegroundColour::kGreen:        seq = "\033[22;32m"; break;
      case ForegroundColour::kCyan:         seq = "\033[22;36m"; break;
      case ForegroundColour::kRed:          seq = "\033[22;31m"; break;
      case ForegroundColour::kMagenta:      seq = "\033[22;35m"; break;
      case ForegroundColour::kBrown:        seq = "\033[22;33m"; break;
      case ForegroundColour::kGrey:         seq = "\033[22;37m"; break;
      case ForegroundColour::kDarkGrey:     seq = "\033[01;30m"; break;
      case ForegroundColour::kLightBlue :   seq = "\033[01;34m"; break;
      case ForegroundColour::kLightGreen:   seq = "\033[01;32m"; break;
      case ForegroundColour::kLightCyan:    seq = "\033[01;36m"; break;
      case ForegroundColour::kLightRed:     seq = "\033[01;31m"; break;
      case ForegroundColour::kLightMagenta: seq = "\033[01;35m"; break;
      case ForegroundColour::kYellow:       seq = "\033[01;33m"; break;
      case ForegroundColour::kWhite:        seq = "\033[01;37m"; break;
      default:                              seq = "";            break;
    }

    fputs(seq, stream);
#endif
}

void SetConsoleColour(FILE* stream, BackgroundColour colour) {
#ifdef _WIN32
  HANDLE hdl =
      GetStdHandle(stream == stdout ? STD_OUTPUT_HANDLE : STD_ERROR_HANDLE);
  CONSOLE_SCREEN_BUFFER_INFO csbi;
  GetConsoleScreenBufferInfo(hdl, &csbi);
  SetConsoleTextAttribute(
      hdl, (csbi.wAttributes & 0xFF0F) | (static_cast<WORD>(colour) << 4));
#else
    if (!isatty(stream == stdout ? STDOUT_FILENO : STDERR_FILENO)) {
        return;
    }

    const char* seq;
    switch (colour) {
      case BackgroundColour::kBlack:   seq = "\033[40m"; break;
      case BackgroundColour::kRed:     seq = "\033[41m"; break;
      case BackgroundColour::kGreed:   seq = "\033[42m"; break;
      case BackgroundColour::kYellow:  seq = "\033[43m"; break;
      case BackgroundColour::kBlue:    seq = "\033[44m"; break;
      case BackgroundColour::kMagenta: seq = "\033[45m"; break;
      case BackgroundColour::kCyan:    seq = "\033[46m"; break;
      case BackgroundColour::kWhite:   seq = "\033[47m"; break;
      default:                         seq = "";         break;
    }

    fputs(seq, stream);
#endif
}

void ResetConsoleColours(FILE* stream) {
#ifdef _WIN32
  if (stream == stdout) {
    HANDLE hdl = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hdl, win32_original_stdout_colours);
  }
  else {
    HANDLE hdl = GetStdHandle(STD_ERROR_HANDLE);
    SetConsoleTextAttribute(hdl, win32_original_stderr_colours);
  }
#else
  if (!isatty(stream == stdout ? STDOUT_FILENO : STDERR_FILENO)) {
    return;
  }

  fputs("\033[0m", stream);
#endif
}

}  // namespace utils
