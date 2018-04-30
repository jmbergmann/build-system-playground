#pragma once

#include "../../../config.h"
#include "sink.h"

#include <functional>

namespace objects {
namespace detail {
namespace log {

class HookSink : public Sink {
 public:
  typedef std::function<void(Verbosity, const utils::Timestamp&, int,
                             const char*, int, const std::string&, const char*)>
      HookFn;

  HookSink(Verbosity verbosity, HookFn fn);

  virtual void WriteEntry(Verbosity severity, const utils::Timestamp& timestamp,
                          int tid, const char* file, int line,
                          const std::string& component,
                          const char* msg) override;

 private:
  const HookFn hook_fn_;
};

typedef std::unique_ptr<HookSink> HookSinkPtr;

}  // namespace log
}  // namespace detail
}  // namespace objects
