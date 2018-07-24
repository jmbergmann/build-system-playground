#pragma once

#include "internal/library.h"
#include "internal/error_code_helpers.h"

#include <chrono>
#include <cassert>
#include <sstream>
#include <string>
#include <iomanip>

namespace yogi {
namespace internal {

template <typename Fn, typename... Args>
inline void* CallApiCreate(Fn fn, Args&&... args) {
  void* handle;
  int res = fn(&handle, std::forward<Args>(args)...);
  CheckErrorCode(res);
  return handle;
}

}  // namespace internal

YOGI_DEFINE_API_FN(int, YOGI_Destroy, (void* object))

//! Base class for all "creatable" objects.
//!
//! "Creatable" Yogi objects are objects that get instantiated and live until
//! they are destroyed by the user.
class Object {
 public:
  //! Constructs the object.
  //!
  //! \param handle Yogi handle representing the object.
  Object(void* handle) : handle_(handle) {}

  virtual ~Object() {
    int res = internal::YOGI_Destroy(handle_);
    assert(res == 0);
  }

  //! Returns the name of the object type (e.g. "Context").
  //!
  //! \returns Object type name.
  virtual const std::string& GetObjectTypeName() const = 0;

  //! Returns a human-readable string identifying the object.
  //!
  //! \returns Human-readable string identifying the object.
  std::string ToString() const {
    std::stringstream ss;
    ss << GetObjectTypeName() << " [" << std::hex
       << reinterpret_cast<long long>(handle_) << "]";
    return ss.str();
  }

 private:
  void* handle_;
};

}  // namespace yogi
