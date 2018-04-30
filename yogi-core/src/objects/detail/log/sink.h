#pragma once

#include "../../../config.h"
#include "../../../utils/timestamp.h"

#include <memory>

namespace objects {
namespace detail {
namespace log {

class Sink {
 public:
  enum Verbosity {
    kVerbosityFatal = YOGI_VB_FATAL,
    kVerbosityError = YOGI_VB_ERROR,
    kVerbosityWarning = YOGI_VB_WARNING,
    kVerbosityInfo = YOGI_VB_INFO,
    kVerbosityDebug = YOGI_VB_DEBUG,
    kVerbosityTrace = YOGI_VB_TRACE,
  };

  Sink(Verbosity verbosity) : verbosity_(verbosity) {}
  virtual ~Sink() {}

  void Publish(Verbosity severity, const utils::Timestamp& timestamp, int tid,
               const char* file, int line, const std::string& component,
               const char* msg);

 protected:
  virtual void WriteEntry(Verbosity severity, const utils::Timestamp& timestamp,
                          int tid, const char* file, int line,
                          const std::string& component, const char* msg) = 0;

 private:
  const Verbosity verbosity_;
};

typedef std::unique_ptr<Sink> SinkPtr;

}  // namespace log
}  // namespace detail
}  // namespace objects
