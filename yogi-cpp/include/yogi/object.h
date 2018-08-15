#pragma once

#include "internal/string_conversion.h"
#include "internal/library.h"
#include "internal/error_code_helpers.h"

#include <chrono>
#include <cassert>
#include <string>

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

YOGI_DEFINE_API_FN(int, YOGI_FormatObject,
                   (void* object, char* str, int strsize, const char* objfmt,
                    const char* nullstr))
YOGI_DEFINE_API_FN(int, YOGI_Destroy, (void* object))

/// Base class for all "creatable" objects.
///
/// "Creatable" Yogi objects are objects that get instantiated and live until
/// they are destroyed by the user.
class Object {
 public:
  virtual ~Object() {
    if (handle_ == nullptr) return;
    int res = internal::YOGI_Destroy(handle_);
    assert(res == 0);
  }

  /// Creates a string describing the object.
  ///
  /// The \p objfmt parameter describes the format of the string. The following
  /// placeholders are supported:
  ///  - *$T*: Type of the object (e.g. Branch)
  ///  - *$x*: Handle of the object in lower-case hex notation
  ///  - *$X*: Handle of the object in upper-case hex notation
  ///
  /// By default, the object will be formatted in the format "Branch [44fdde]"
  /// with the hex value in brackets being the object's handle.
  ///
  /// If, for any reason, the object's handle is NULL, this function returns
  /// the nullstr parameter value ("INVALID HANDLE" by default).
  ///
  /// \tparam FmtString Type of the \p fmt string.
  /// \tparam NullString Type of the \p nullstr string.
  ///
  /// \param fmt     Format of the string
  /// \param nullstr String to use if the object's handle is NULL
  ///
  /// \returns Formatted string.
  template <typename FmtString = char*, typename NullString = char*>
  std::string Format(const FmtString& fmt = nullptr,
                     const NullString& nullstr = nullptr) const {
    char str[128];
    int res = internal::YOGI_FormatObject(
        handle_, str, sizeof(str), internal::StringToCoreString(fmt),
        internal::StringToCoreString(nullstr));
    internal::CheckErrorCode(res);
    return str;
  }

  /// Returns a human-readable string identifying the object.
  ///
  /// \returns Human-readable string identifying the object.
  virtual std::string ToString() const {
    return Format();
  }

 protected:
  /// Constructs the object.
  ///
  /// \param handle Yogi handle representing the object.
  Object(void* handle) : handle_(handle) {}

  /// Returns the Yogi handle for this object.
  ///
  /// \returns The Yogi handle for this object.
  void* GetHandle() const { return handle_; }

 private:
  void* handle_;
};

}  // namespace yogi
