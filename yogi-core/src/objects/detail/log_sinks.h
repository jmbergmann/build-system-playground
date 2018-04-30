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

  LogSink(Verbosity verbosity) : verbosity_(verbosity) {}
  virtual ~LogSink() {}

  void Put(Verbosity verbosity, const utils::Timestamp& timestamp,
           int tid, const char* file, int line,
           const std::string& component, const char* msg);

  virtual void Write(Verbosity verbosity, const utils::Timestamp& timestamp,
                     int tid, const char* file, int line,
                     const std::string& component, const char* msg) = 0;

 private:
  const Verbosity& verbosity_;
};

typedef std::unique_ptr<LogSink> LogSinkPtr;

class ConsoleLogSink : public LogSink {
 public:
  ConsoleLogSink(FILE* stream, bool colour, std::string fmt,
                 Verbosity verbosity);

  virtual void Write(Verbosity verbosity, const utils::Timestamp& timestamp,
                     int tid, const char* file, int line,
                     const std::string& component, const char* msg) override;

 private:
  FILE* const stream_;
  const bool colour_;
  const std::string fmt_;
};

typedef std::unique_ptr<ConsoleLogSink> ConsoleLogSinkPtr;

class HookLogSink : public LogSink {
 public:
  typedef std::function<void(Verbosity, const utils::Timestamp&, int,
                             const char*, int, const std::string&, const char*)>
      HookFn;

  HookLogSink(HookFn fn, Verbosity verbosity);

  virtual void Write(Verbosity verbosity, const utils::Timestamp& timestamp,
                     int tid, const char* file, int line,
                     const std::string& component, const char* msg) override;

 private:
  const HookFn hook_fn_;
};

typedef std::unique_ptr<HookLogSink> HookLogSinkPtr;

class FileLogSink : public LogSink {
 public:
  FileLogSink(std::string filename, std::string fmt, Verbosity verbosity);

  virtual void Write(Verbosity verbosity, const utils::Timestamp& timestamp,
                     int tid, const char* file, int line,
                     const std::string& component, const char* msg) override;

 private:
  const std::string filename_;
  const std::string fmt_;
};

typedef std::unique_ptr<FileLogSink> FileLogSinkPtr;

}  // namespace detail
}  // namespace objects
