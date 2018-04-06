#pragma once

#include "../config.h"

#include <memory>

namespace api {

enum class ObjectType {
  kContext,
};

class ExposedObject : public std::enable_shared_from_this<ExposedObject> {
 public:
  ExposedObject() = default;
  virtual ~ExposedObject() = default;

  virtual ObjectType type() const = 0;

  template <typename T>
  std::shared_ptr<T> make_ptr() {
    return std::dynamic_pointer_cast<T>(shared_from_this());
  }

 private:
  // noncopyable
  ExposedObject(const ExposedObject&) = delete;
  void operator=(const ExposedObject&) = delete;
};

class ObjectRegister {
 public:

};

}  // namespace api
