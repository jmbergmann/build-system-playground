#include "helpers.h"

#include <string.h>
#include <boost/algorithm/string.hpp>
#include <regex>

bool IsExactlyOneBitSet(int bit_field) {
  auto x = static_cast<unsigned int>(bit_field);
  return x && !(x & (x - 1));
}

bool IsTimeFormatValid(const std::string& fmt) {
  if (fmt.empty()) return false;

  std::regex re("%([^YmdFHMST369]|$)");
  return !std::regex_search(fmt, re);
}

bool IsLogFormatValid(std::string fmt) {
  if (fmt.empty()) return false;

  boost::replace_all(fmt, "$$", "");
  std::regex re("\\$([^tPTsmflc<>]|$)");
  return !std::regex_search(fmt, re);
}

void CopyUuidToUserBuffer(const boost::uuids::uuid& uuid, void* buffer) {
  if (buffer == nullptr) return;
  std::memcpy(buffer, &uuid, uuid.size());
}

bool CopyStringToUserBuffer(const std::string& str, char* buffer,
                            int buffer_size) {
  if (buffer == nullptr) return true;

  auto n = std::min(str.size() + 1, static_cast<std::size_t>(buffer_size));
  std::strncpy(buffer, str.c_str(), n);
  if (str.size() + 1 > n) {
    buffer[buffer_size - 1] = '\0';
    return false;
  }

  return true;
}
