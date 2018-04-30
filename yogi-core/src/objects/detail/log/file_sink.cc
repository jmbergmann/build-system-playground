#include "file_sink.h"

namespace objects {
namespace detail {
namespace log {

FileSink::FileSink(Verbosity verbosity, std::string filename, std::string time_fmt,
                         std::string fmt)
    : TextBasedSink(verbosity, time_fmt, fmt, true) {}

void FileSink::WritePartialOutput(const std::string& str) {}

void FileSink::Flush() {}

}  // namespace log
}  // namespace detail
}  // namespace objects
