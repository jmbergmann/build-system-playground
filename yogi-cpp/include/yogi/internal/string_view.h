#ifndef YOGI_INTERNAL_STRING_VIEW_H
#define YOGI_INTERNAL_STRING_VIEW_H

#include <string>

namespace yogi {
namespace internal {

class StringView {
 public:
  StringView() : s_(nullptr) {}
  StringView(const std::string& s) : s_(s.c_str()) {}
  StringView(const char* s) : s_(s) {};

  operator const char* () const { return s_; }

 private:
  const char* const s_;
};

}  // namespace internal
}  // namespace yogi

#endif  // YOGI_INTERNAL_STRING_VIEW_H
