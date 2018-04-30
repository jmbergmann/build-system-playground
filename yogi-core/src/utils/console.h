#pragma once

#include "../config.h"

#include <string>
#include <stdio.h>

namespace utils {

enum class ForegroundColour {
  kBlack,
  kBlue,
  kGreen,
  kCyan,
  kRed,
  kMagenta,
  kBrown,
  kGrey,
  kDarkGrey,
  kLightBlue,
  kLightGreen,
  kLightCyan,
  kLightRed,
  kLightMagenta,
  kYellow,
  kWhite,
};

enum class BackgroundColour {
  kBlack   = static_cast<int>(ForegroundColour::kBlack),
  kRed     = static_cast<int>(ForegroundColour::kRed),
  kGreen   = static_cast<int>(ForegroundColour::kGreen),
  kYellow  = static_cast<int>(ForegroundColour::kYellow),
  kBlue    = static_cast<int>(ForegroundColour::kBlue),
  kMagenta = static_cast<int>(ForegroundColour::kMagenta),
  kCyan    = static_cast<int>(ForegroundColour::kCyan),
  kWhite   = static_cast<int>(ForegroundColour::kWhite),
};

void SetConsoleTitle(FILE* stream, const std::string& title);
void SetConsoleColour(FILE* stream, ForegroundColour colour);
void SetConsoleColour(FILE* stream, BackgroundColour colour);
void ResetConsoleColours(FILE* stream);

}  // namespace utils
