#include "file_sink.h"
#include "../../../api/error.h"

namespace objects {
namespace detail {
namespace log {

FileSink::FileSink(Verbosity verbosity, std::string filename,
                   std::string time_fmt, std::string fmt)
    : TextBasedSink(verbosity, time_fmt, fmt, true),
      filename_(utils::Timestamp::Now().ToFormattedString(filename)) {
  file_.open(filename_, std::ios::out | std::ios::trunc);
  if (!file_.is_open()) {
    throw api::Error(YOGI_ERR_OPEN_FILE_FAILED);
  }
}

void FileSink::WritePartialOutput(const std::string& str) { file_ << str; }

void FileSink::Flush() { file_ << std::flush; }

}  // namespace log
}  // namespace detail
}  // namespace objects
