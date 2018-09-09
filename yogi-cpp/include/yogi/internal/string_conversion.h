#ifndef YOGI_INTERNAL_STRING_CONVERSION_H
#define YOGI_INTERNAL_STRING_CONVERSION_H

#include <string>

namespace yogi {
namespace internal {

inline const char* ToCoreString(const char* s) { return s; }

inline const char* ToCoreString(const std::string& s) { return s.c_str(); }

}  // namespace internal
}  // namespace yogi

#endif  // YOGI_INTERNAL_STRING_CONVERSION_H
