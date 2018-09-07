#pragma once

#include "../config.h"
#include "../../../3rd_party/nlohmann/json.hpp"

#include <chrono>
#include <string>
#include <boost/uuid/uuid.hpp>

inline std::chrono::nanoseconds ConvertDuration(long long duration) {
  return duration == -1 ? std::chrono::nanoseconds::max()
                        : std::chrono::nanoseconds(duration);
}

std::chrono::nanoseconds ExtractDuration(const nlohmann::json& json,
                                         const char* key,
                                         long long defaultValue);

template <typename Enum>
inline Enum ConvertFlags(int flags, Enum default_flags) {
  return flags ? static_cast<Enum>(flags) : default_flags;
}

bool IsExactlyOneBitSet(int bit_field);
bool IsTimeFormatValid(const std::string& fmt);
bool IsLogFormatValid(std::string fmt);
void CopyUuidToUserBuffer(const boost::uuids::uuid& uuid, void* buffer);
bool CopyStringToUserBuffer(const std::string& str, char* buffer,
                            int buffer_size);
