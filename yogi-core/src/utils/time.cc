#include "time.h"

#include <boost/date_time/posix_time/posix_time_io.hpp>
#include <boost/date_time/local_time_adjustor.hpp>
#include <boost/date_time/c_local_time_adjustor.hpp>
#include <sstream>

namespace utils {

boost::posix_time::ptime GetCurrentUtcTime() {
  return boost::posix_time::ptime(
      boost::gregorian::day_clock::universal_day(),
      boost::posix_time::microsec_clock::universal_time().time_of_day());
}

std::string TimeToJavaScriptString(const boost::posix_time::ptime& time) {
  auto facet = new boost::posix_time::time_facet();
  facet->format("%Y-%m-%dT%T.%fZ");

  std::ostringstream oss;
  oss.imbue(std::locale(std::locale::classic(), facet));
  oss << time;

  auto s = oss.str();
  s.erase(s.end() - 4, s.end() - 1);  // Remove microseconds from string
  return s;
}

}  // namespace utils
