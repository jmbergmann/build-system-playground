#pragma once

#include "../../config.h"
#include "../../utils/timestamp.h"

#include <memory>

namespace objects {
namespace detail {

class LogSink {
 public:
  enum Verbosity {
    kVerbosityFatal   = YOGI_VB_FATAL,
    kVerbosityError   = YOGI_VB_ERROR,
    kVerbosityWarning = YOGI_VB_WARNING,
    kVerbosityInfo    = YOGI_VB_INFO,
    kVerbosityDebug   = YOGI_VB_DEBUG,
    kVerbosityTrace   = YOGI_VB_TRACE,
  };
};

class ConsoleLogSink : public LogSink {
 public:
  ConsoleLogSink(FILE* stream, bool colour, std::string fmt,
                 Verbosity verbosity);
};

typedef std::unique_ptr<ConsoleLogSink> ConsoleLogSinkPtr;

class HookLogSink : public LogSink {
 public:
  typedef std::function<void(Verbosity, const utils::Timestamp&, int,
                             const char*, int, const std::string&, const char*)>
      HookFn;

  HookLogSink(HookFn fn, Verbosity verbosity);
};

typedef std::unique_ptr<HookLogSink> HookLogSinkPtr;

class FileLogSink : public LogSink {
 public:
  FileLogSink(std::string filename, std::string fmt, Verbosity verbosity);
};

typedef std::unique_ptr<FileLogSink> FileLogSinkPtr;

}  // namespace detail
}  // namespace objects
