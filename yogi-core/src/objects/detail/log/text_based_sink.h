#pragma once

#include "../../../config.h"
#include "sink.h"

#include <string>

namespace objects {
namespace detail {
namespace log {

class TextBasedSink : public Sink {
 public:
  TextBasedSink(Verbosity verbosity, std::string time_fmt, std::string fmt,
                bool ignore_colour);

 protected:
  virtual void WriteEntry(Verbosity severity, const utils::Timestamp& timestamp,
                          int tid, const char* file, int line,
                          const std::string& component,
                          const char* msg) override;

  virtual void WritePartialOutput(const std::string& str) = 0;
  virtual void SetOutputColour(bool on) {}

 private:
  const std::string time_fmt_;
  const std::string fmt_;
  const bool ignore_colour_;
};

typedef std::unique_ptr<Sink> SinkPtr;

}  // namespace log
}  // namespace detail
}  // namespace objects
