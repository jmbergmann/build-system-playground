#pragma once

#include "internal/library.h"
#include "io.h"

#include <cassert>

namespace yogi {

/// Levels of how verbose logging output is.
///
/// The term "severity" is refers to the same type.
enum class Verbosity {
  /// Fatal errors are errors that require a process restart.
  kFatal =  0,

  /// Errors that the system can recover from.
  kError = 1,

  /// >Warnings.
  kWarning = 2,

  /// Useful general information about the system state.
  kInfo = 3,

  /// Information for debugging.
  kDebug = 4,

  /// Detailed debugging information.
  kTrace = 5,
};

template <>
inline std::string ToString<Verbosity>(const Verbosity& vb) {
  switch (vb) {
    YOGI_TO_STRING_ENUM_CASE(Verbosity, kFatal)
    YOGI_TO_STRING_ENUM_CASE(Verbosity, kError)
    YOGI_TO_STRING_ENUM_CASE(Verbosity, kWarning)
    YOGI_TO_STRING_ENUM_CASE(Verbosity, kInfo)
    YOGI_TO_STRING_ENUM_CASE(Verbosity, kDebug)
    YOGI_TO_STRING_ENUM_CASE(Verbosity, kTrace)
  }

  bool should_never_get_here = false;
  assert(should_never_get_here);
  return {};
}

}  // namespace yogi
