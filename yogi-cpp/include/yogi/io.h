#ifndef YOGI_IO_H
#define YOGI_IO_H

#include "internal/sfinae.h"

#include <string>

#define YOGI_TO_STRING_ENUM_CASE(enum_type, enum_element) \
  case enum_type::enum_element:                           \
    return #enum_element;

#define YOGI_TO_STRING_FLAG_APPENDER(var, enum_type, enum_element) \
  if ((var & enum_type::enum_element) != enum_type::kNone) {       \
    s += " | " #enum_element;                                      \
  }

namespace yogi {

YOGI_DEFINE_SFINAE_METHOD_TESTER(HasToStringMethod,
                                 .ToString() == std::string())

/// Converts a given Yogi enum value or object to a string.
///
/// \tparam T Type of the enum or object.
///
/// \param printable The object to convert to a string.
///
/// \returns Human-readable string name or description of the object.
template <typename T>
inline std::string ToString(const T& printable) {
  static_assert(internal::HasToStringMethod<T>::value,
                "T has no usable ToString() method.");

  return printable.ToString();
}

}  // namespace yogi

#endif  // YOGI_IO_H
