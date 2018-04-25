#include "../config.h"

#include <boost/date_time/posix_time/posix_time.hpp>

namespace utils {

boost::posix_time::ptime GetCurrentUtcTime();
std::string TimeToJavaScriptString(const boost::posix_time::ptime& time);

}  // namespace utils
