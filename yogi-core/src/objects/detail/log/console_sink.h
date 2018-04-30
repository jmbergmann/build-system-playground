#pragma once

#include "../../../config.h"
#include "text_based_sink.h"

namespace objects {
namespace detail {
namespace log {

class ConsoleSink : public TextBasedSink {
 public:
  ConsoleSink(Verbosity verbosity, FILE* stream, bool colour, std::string time_fmt,
                 std::string fmt);

 protected:
  virtual void WritePartialOutput(const std::string& str) override;
  virtual void SetOutputColours(Verbosity severity) override;
  virtual void ResetOutputColours() override;
  virtual void Flush() override;

 private:
  FILE* const stream_;
  const bool colour_;
};

typedef std::unique_ptr<ConsoleSink> ConsoleSinkPtr;

}  // namespace log
}  // namespace detail
}  // namespace objects
