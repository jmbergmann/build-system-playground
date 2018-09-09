#pragma once

#include "../config.h"

#include <nlohmann/json.hpp>
#include <boost/uuid/uuid.hpp>
#include <chrono>
#include <string>

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
