#pragma once

#include "../../../config.h"
#include "text_based_log_sink.h"

#include <fstream>

namespace objects {
namespace detail {

class FileLogSink : public TextBasedLogSink {
 public:
  FileLogSink(Verbosity verbosity, std::string filename, std::string time_fmt,
              std::string fmt);

  const std::string& GetGeneratedFilename() const { return filename_; }

 protected:
  virtual void WritePartialOutput(const std::string& str) override;
  virtual void Flush() override;

 private:
  const std::string filename_;
  std::ofstream file_;
};

typedef std::unique_ptr<FileLogSink> FileLogSinkPtr;

}  // namespace detail
}  // namespace objects
