#pragma once

#include "../config.h"

#include <memory>

namespace api {

class ExposedObject : public std::enable_shared_from_this<ExposedObject> {
 public:
  ExposedObject() = default;
  virtual ~ExposedObject() = default;

  template <typename T>
  std::shared_ptr<T> make_ptr() {
    return std::dynamic_pointer_cast<T>(shared_from_this());
  }

 private:
  // noncopyable
  ExposedObject(const ExposedObject&) = delete;
  void operator=(const ExposedObject&) = delete;
};

}  // namespace api
