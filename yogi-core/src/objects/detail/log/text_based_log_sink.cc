#include "text_based_log_sink.h"
#include "../../../utils/system.h"

#include <sstream>

namespace objects {
namespace detail {

TextBasedLogSink::TextBasedLogSink(Verbosity verbosity, std::string time_fmt,
                                   std::string fmt, bool ignore_colour)
    : LogSink(verbosity),
      time_fmt_(time_fmt),
      fmt_(fmt),
      ignore_colour_(ignore_colour) {}

void TextBasedLogSink::WriteEntry(Verbosity severity,
                                  const utils::Timestamp& timestamp, int tid,
                                  const char* file, int line,
                                  const std::string& component,
                                  const char* msg) {
  std::stringstream ss;
  bool colour_cleared = true;

  std::string::size_type old_pos = 0;
  std::string::size_type pos = fmt_.find('$');
  while (pos != std::string::npos) {
    if (pos > old_pos) {
      ss.write(fmt_.c_str() + old_pos,
	           static_cast<std::streamsize>(pos - old_pos));
    }

    switch (fmt_[pos + 1]) {
      case 't':
        ss << timestamp.ToFormattedString(time_fmt_);
        break;

      case 'P':
        ss << utils::GetProcessId();
        break;

      case 'T':
        ss << tid;
        break;

      case 's':
        switch (severity) {
          case kFatal:
            ss << "FAT";
            break;
          case kError:
            ss << "ERR";
            break;
          case kWarning:
            ss << "WRN";
            break;
          case kInfo:
            ss << "IFO";
            break;
          case kDebug:
            ss << "DBG";
            break;
          case kTrace:
            ss << "TRC";
            break;
          default:
            YOGI_NEVER_REACHED;
            break;
        }
        break;

      case 'm':
        ss << msg;
        break;

      case 'f':
        ss << file;
        break;

      case 'l':
        ss << line;
        break;

      case 'c':
        ss << component;
        break;

      case '<':
        if (!ignore_colour_ && colour_cleared) {
          if (ss.rdbuf()->in_avail()) {
            WritePartialOutput(ss.str());
            ss.str(std::string());
          }

          SetOutputColours(severity);
          colour_cleared = false;
        }
        break;

      case '>':
        if (!ignore_colour_ && !colour_cleared) {
          if (ss.rdbuf()->in_avail()) {
            WritePartialOutput(ss.str());
            ss.str(std::string());
          }

          ResetOutputColours();
          colour_cleared = true;
        }
        break;

      case '$':
        ss << '$';
        break;
    }

    old_pos = pos + 2;  // skip placeholder
    pos = fmt_.find('$', old_pos);
  }

  ss << fmt_.substr(old_pos);
  ss << std::endl;
  WritePartialOutput(ss.str());
  if (!colour_cleared) {
    ResetOutputColours();
  }

  Flush();
}

}  // namespace detail
}  // namespace objects
