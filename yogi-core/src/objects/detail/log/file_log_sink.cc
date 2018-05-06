#include "file_log_sink.h"
#include "../../../api/error.h"

namespace objects {
namespace detail {

FileLogSink::FileLogSink(Verbosity verbosity, std::string filename,
                         std::string time_fmt, std::string fmt)
    : TextBasedLogSink(verbosity, time_fmt, fmt, true),
      filename_(utils::Timestamp::Now().ToFormattedString(filename)) {
  file_.open(filename_, std::ios::out | std::ios::trunc);
  if (!file_.is_open()) {
    throw api::Error(YOGI_ERR_OPEN_FILE_FAILED);
  }
}

void FileLogSink::WritePartialOutput(const std::string& str) { file_ << str; }

void FileLogSink::Flush() { file_ << std::flush; }

}  // namespace detail
}  // namespace objects
