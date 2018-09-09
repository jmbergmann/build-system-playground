#ifndef YOGI_INTERNAL_JSON_VIEW_H
#define YOGI_INTERNAL_JSON_VIEW_H

#include "../json.h"

#include <string>

namespace yogi {
namespace internal {

class JsonView {
 public:
  JsonView() : s_(nullptr) {}
  JsonView(const std::string& s) : s_(s.c_str()) {}
  JsonView(const char* s) : s_(s) {};
  JsonView(const Json& json) : tmp_(json.dump()), s_(tmp_.c_str()) {};

  operator const char* () const { return s_; }

 private:
  const std::string tmp_;
  const char* const s_;
};

}  // namespace internal
}  // namespace yogi

#endif  // YOGI_INTERNAL_JSON_VIEW_H
