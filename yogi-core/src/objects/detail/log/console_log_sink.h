#pragma once

#include "../../../config.h"
#include "text_based_log_sink.h"

namespace objects {
namespace detail {

class ConsoleLogSink : public TextBasedLogSink {
 public:
  ConsoleLogSink(Verbosity verbosity, FILE* stream, bool color,
                 std::string time_fmt, std::string fmt);

 protected:
  virtual void WritePartialOutput(const std::string& str) override;
  virtual void SetOutputColours(Verbosity severity) override;
  virtual void ResetOutputColours() override;
  virtual void Flush() override;

 private:
  FILE* const stream_;
  const bool colour_;
};

typedef std::unique_ptr<ConsoleLogSink> ConsoleLogSinkPtr;

}  // namespace detail
}  // namespace objects
