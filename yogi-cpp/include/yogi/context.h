#pragma once

#include "object.h"

namespace yogi {

YOGI_DEFINE_API_FN(int, YOGI_ContextCreate, (void** context))

//! Scheduler for the execution of asynchronous operations.
//!
//! Once an asynchronous operation finishes, the corresponding handler function
//! is added to the context's event queue and executed through once of the
//! Poll... or Run... functions.
class Context : public Object {
 public:
  //! Constructs the context.
  Context() : Object(internal::CallApiCreate(internal::YOGI_ContextCreate)) {}

  virtual const std::string& GetObjectTypeName() const {
    static std::string s = "Context";
    return s;
  }
};

}  // namespace yogi
