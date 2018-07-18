#pragma once

#include "internal/library.h"


namespace yogi {
namespace internal {
namespace api {

static const char* GetVersion() {
  static auto fn = Library::GetFunctionAddress<const char* (*)()>("YOGI_GetVersion");
  return fn();
}

}  // namespace api
}  // namespace internal

inline const std::string& GetVersion() {
  static std::string s = internal::api::GetVersion();
  return s;
}

}  // namespace yogi
