#include "timestamp.h"

#include <boost/algorithm/string.hpp>
#include <time.h>
#include <stdio.h>
#include <ctime>
#include <iomanip>
#include <sstream>

namespace utils {

Timestamp Timestamp::Now() {
  return Timestamp(std::chrono::time_point_cast<std::chrono::nanoseconds>(
      std::chrono::system_clock::now()));
}

std::string Timestamp::ToFormattedString(std::string fmt) const {
  auto time = std::chrono::system_clock::to_time_t(
      std::chrono::time_point_cast<std::chrono::system_clock::duration>(time_));

  struct tm buf;
#ifdef _WIN32
  gmtime_s(&buf, &time);
#else
  gmtime_r(&time, &buf);
#endif

  char tmp[4];
  sprintf(tmp, "%03i", static_cast<int>(MillisecondsFraction()));
  boost::replace_all(fmt, "%3", tmp);
  sprintf(tmp, "%03i", static_cast<int>(MicrosecondsFraction()));
  boost::replace_all(fmt, "%6", tmp);
  sprintf(tmp, "%03i", static_cast<int>(NanosecondsFraction()));
  boost::replace_all(fmt, "%9", tmp);

  std::stringstream ss;
  ss << std::put_time(&buf, fmt.c_str());
  return ss.str();
}

std::string Timestamp::ToJavaScriptString() const {
  return ToFormattedString("%FT%T.%3Z");
}

}  // namespace utils
