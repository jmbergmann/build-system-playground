/*
 * This file is part of the Yogi distribution https://github.com/yohummus/yogi.
 * Copyright (c) 2018 Johannes Bergmann.
 *
 * This library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library. If not, see <http://www.gnu.org/licenses/>.
 */

#include "helpers.h"
#include "../api/errors.h"

#include <string.h>
#include <boost/algorithm/string.hpp>
#include <regex>

std::chrono::nanoseconds ExtractDuration(const nlohmann::json& json,
                                         const char* key,
                                         long long defaultValue) {
  float seconds = json.value(key, static_cast<float>(defaultValue) / 1e9f);
  if (seconds == -1) {
    return (std::chrono::nanoseconds::max)();
  } else if (seconds < 0) {
    return (std::chrono::nanoseconds::max)();
    // TODO: Throw exception with additional information
  }

  auto ns = static_cast<long long>(seconds * 1e9f);
  return std::chrono::nanoseconds(ns);
}

std::vector<std::string> ExtractArrayOfStrings(const nlohmann::json& json,
                                               const char* key,
                                               const char* default_val) {
  std::vector<std::string> v;
  nlohmann::json json_vec;

  if (json.count(key)) {
    json_vec = json[key];
  } else {
    json_vec = nlohmann::json::parse(default_val);
  }

  if (!json_vec.is_array()) {
    return v;
    // TODO: Throw exception
  }

  for (auto& elem : json_vec) {
    if (!elem.is_string()) {
      continue;
      // TODO: Throw exception
    }

    v.push_back(elem.get<std::string>());
  }

  return v;
}

int ExtractLimitedInt(const nlohmann::json& json, const char* key,
                      int default_val, int min_val, int max_val) {
  int val = json.value(key, default_val);
  if (min_val > val || val > max_val) {
    throw api::DescriptiveError(YOGI_ERR_INVALID_PARAM)
        << "Property \"" << key << "\" is out of range. Allowed range is "
        << min_val << " to " << max_val << ".";
  }

  return val;
}

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

nlohmann::json ParseBranchProps(const char* props, const char* section) {
  auto properties = nlohmann::json::object();
  if (props) {
    try {
      properties = nlohmann::json::parse(props);
    } catch (const nlohmann::json::exception& e) {
      throw api::DescriptiveError(YOGI_ERR_PARSING_JSON_FAILED)
          << "Could not parse JSON string: " << e.what();
    }

    if (section) {
      nlohmann::json::json_pointer jp;

      try {
        jp = nlohmann::json::json_pointer(section);
      } catch (const nlohmann::json::exception& e) {
        throw api::DescriptiveError(YOGI_ERR_INVALID_PARAM)
            << "Could not parse JSON pointer: " << e.what();
      }

      properties = properties[jp];
      if (!properties.is_object()) {
        throw api::DescriptiveError(YOGI_ERR_PARSING_JSON_FAILED)
            << "Could not find section \"" << section
            << "\" in branch properties.";
      }
    }
  }

  return properties;
}
