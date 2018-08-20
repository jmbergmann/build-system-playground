#pragma once

#include "../errors.h"
#include "../duration.h"
#include "../timestamp.h"

#include <string>
#include <cstring>

namespace yogi {
namespace internal {

template <int N>
inline std::string::const_iterator FindElementInJson(const std::string& json,
                                                const char (&name)[N]) {
  char key[N + 3];
  key[0] = '"';
  std::memcpy(key + 1, name, N - 1);
  key[N] = '"';
  key[N + 1] = ':';
  key[N + 2] = '\0';

  auto pos = json.find(key);
  if (pos == std::string::npos) {
    throw DescriptiveFailureException(
        ErrorCode::kParsingJsonFailed,
        std::string("Could not find key '") + name + "' from " + json);
  }

  return json.begin() + pos + N + 2;
}

template <int N>
inline int ExtractIntFromJson(const std::string& json, const char (&name)[N]) {
  auto it = FindElementInJson(json, name);

  try {
    return std::stoi(std::string(it, json.end()));
  } catch (const std::exception& e) {
    throw DescriptiveFailureException(
        ErrorCode::kParsingJsonFailed,
        std::string("Could not extract int '") + name + "': " + e.what());
  }
}

template <int N>
inline float ExtractFloatFromJson(const std::string& json,
                                  const char (&name)[N]) {
  auto it = FindElementInJson(json, name);

  try {
    return std::stof(std::string(it, json.end()));
  } catch (const std::exception& e) {
    throw DescriptiveFailureException(
        ErrorCode::kParsingJsonFailed,
        std::string("Could not extract float '") + name + "': " + e.what());
  }
}

template <int N>
inline std::string ExtractStringFromJson(const std::string& json,
                                         const char (&name)[N]) {
  auto it = FindElementInJson(json, name);

  if (it == json.end() || *it != '"') {
    throw DescriptiveFailureException(
        ErrorCode::kParsingJsonFailed,
        std::string("Could not extract string '") + name +
            "': Start token \" not found");
  }

  std::string s;
  bool escaped = false;
  for (++it; it != json.end(); ++it) {
    if (escaped) {
      switch (*it) {
        case 'b':
          s += '\b';
          break;

        case 'f':
          s += '\f';
          break;

        case 'n':
          s += '\n';
          break;

        case 'r':
          s += '\r';
          break;

        case 't':
          s += '\t';
          break;

        default:
          s += *it;
          break;
      }

      escaped = false;
      continue;
    }

    if (*it == '\\') {
      escaped = true;
      continue;
    }

    if (*it == '"') {
      break;
    }

    s += *it;
  }

  if (it == json.end() || escaped) {
    throw DescriptiveFailureException(
        ErrorCode::kParsingJsonFailed,
        std::string("Could not extract string '") + name +
            "': End token \" not found");
  }

  return s;
}

template <int N>
inline Duration ExtractDurationFromJson(const std::string& json,
                                        const char (&name)[N]) {
  auto seconds = ExtractFloatFromJson(json, name);
  if (seconds < 0 && seconds != -1) {
    throw DescriptiveFailureException(
        ErrorCode::kParsingJsonFailed,
        std::string("Could not extract duration '") + name +
            "': Invalid value");
  }

  if (seconds < 0) {
    return Duration::kInfinity;
  }
  else {
    return Duration::FromSeconds(seconds);
  }
}

template <int N>
inline Timestamp ExtractTimestampFromJson(const std::string& json,
                                          const char (&name)[N]) {
  auto s = ExtractStringFromJson(json, name);
  return Timestamp::Parse(s);
}

}  // namespace internal
}  // namespace yogi
