#pragma once

#include "logging.h"
#include "internal/conversion.h"
#include "internal/error_code_helpers.h"
#include "internal/library.h"

#include <string>
#include <chrono>

namespace yogi {

YOGI_DEFINE_API_FN(int, YOGI_GetConstant, (void* dest, int constant))

namespace internal {

template <typename T>
inline T GetConstant(int constant) {
  T val;
  int res = YOGI_GetConstant(&val, constant);
  CheckErrorCode(res);
  return val;
}

}  // namespace internal

/// constants built into the Yogi Core library.
struct constants {
  /// Complete Yogi Core version number.
  static const std::string kVersionNumber;

  /// Yogi Core major version number.
  static const int kVersionMajor;

  /// Yogi Core minor version number.
  static const int kVersionMinor;

  /// Yogi Core patch version number.
  static const int kVersionPatch;

  /// Default IP address for advertising.
  static const std::string kDefaultAdvAddress;

  /// Default UDP port for advertising.
  static const int kDefaultAdvPort;

  /// Default time between two advertising messages.
  static const std::chrono::nanoseconds kDefaultAdvInterval;

  /// Default timeout for connections between two branches.
  static const std::chrono::nanoseconds kDefaultConnectionTimeout;

  /// Default verbosity for newly created loggers.
  static const Verbosity kDefaultLoggerVerbosity;

  /// Default format of the time string in log entries.
  static const std::string kDefaultLogTimeFormat;

  /// Default format of a log entry.
  static const std::string kDefaultLogFormat;

  /// Maximum size of a message between two branches.
  static const int kMaxMessageSize;
};

YOGI_WEAK_SYMBOL const std::string constants::kVersionNumber =
    internal::GetConstant<char*>(1);

YOGI_WEAK_SYMBOL const int constants::kVersionMajor =
    internal::GetConstant<int>(2);

YOGI_WEAK_SYMBOL const int constants::kVersionMinor =
    internal::GetConstant<int>(3);

YOGI_WEAK_SYMBOL const int constants::kVersionPatch =
    internal::GetConstant<int>(4);

YOGI_WEAK_SYMBOL const std::string constants::kDefaultAdvAddress =
    internal::GetConstant<char*>(5);

YOGI_WEAK_SYMBOL const int constants::kDefaultAdvPort =
    internal::GetConstant<int>(6);

YOGI_WEAK_SYMBOL const std::chrono::nanoseconds constants::kDefaultAdvInterval =
    internal::CoreDurationToDuration(internal::GetConstant<long long>(7));

YOGI_WEAK_SYMBOL const std::chrono::nanoseconds
    constants::kDefaultConnectionTimeout =
    internal::CoreDurationToDuration(internal::GetConstant<long long>(8));

YOGI_WEAK_SYMBOL const Verbosity constants::kDefaultLoggerVerbosity =
    static_cast<Verbosity>(internal::GetConstant<int>(9));

YOGI_WEAK_SYMBOL const std::string constants::kDefaultLogTimeFormat =
    internal::GetConstant<char*>(10);

YOGI_WEAK_SYMBOL const std::string constants::kDefaultLogFormat =
    internal::GetConstant<char*>(11);

YOGI_WEAK_SYMBOL const int constants::kMaxMessageSize =
    internal::GetConstant<int>(12);

}  // namespace yogi
