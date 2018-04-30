#include "timestamp.h"

#include <time.h>
#include <ctime>
#include <iomanip>
#include <sstream>

namespace utils {

Timestamp Timestamp::Now() {
  return Timestamp(std::chrono::time_point_cast<std::chrono::nanoseconds>(
      std::chrono::system_clock::now()));
}

std::string Timestamp::ToJavaScriptString() const {
  auto time = std::chrono::system_clock::to_time_t(
      std::chrono::time_point_cast<std::chrono::system_clock::duration>(time_));

  struct tm buf;
#ifdef _WIN32
  gmtime_s(&buf, &time);
#else
  gmtime_s(&time, &buf);
#endif

  std::stringstream ss;
  ss << std::put_time(&buf, "%FT%T.") << std::setw(3) << std::setfill('0')
     << MillisecondsFraction();
  ss << 'Z';

  return ss.str();
}

}  // namespace utils
