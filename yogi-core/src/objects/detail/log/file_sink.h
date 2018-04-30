#pragma once

#include "../../../config.h"
#include "text_based_sink.h"

namespace objects {
namespace detail {
namespace log {

class FileSink : public TextBasedSink {
 public:
  FileSink(Verbosity verbosity, std::string filename, std::string time_fmt,
           std::string fmt);

 protected:
  virtual void WritePartialOutput(const std::string& str) override;
  virtual void Flush() override;
};

typedef std::unique_ptr<FileSink> FileSinkPtr;

}  // namespace log
}  // namespace detail
}  // namespace objects
