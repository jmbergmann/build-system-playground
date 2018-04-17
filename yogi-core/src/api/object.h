#pragma once

#include "../config.h"
#include "error.h"

#include <memory>
#include <mutex>
#include <unordered_map>

namespace api {

enum class ObjectType {
  kDummy,  // For testing
  kContext,
};

typedef void* ObjectHandle;

class ExposedObject : public std::enable_shared_from_this<ExposedObject> {
 public:
  ExposedObject() = default;
  virtual ~ExposedObject() = default;

  virtual ObjectType Type() const = 0;

  ObjectHandle Handle() { return static_cast<ObjectHandle>(this); }

  template <typename TO>
  std::shared_ptr<TO> Cast() {
    if (Type() != TO::kType) {
      throw Error(YOGI_ERR_WRONG_OBJECT_TYPE);
    }

    return std::static_pointer_cast<TO>(this->shared_from_this());
  }

  template <>
  std::shared_ptr<ExposedObject> Cast() {
    return std::static_pointer_cast<ExposedObject>(this->shared_from_this());
  }

 private:
  // noncopyable
  ExposedObject(const ExposedObject&) = delete;
  void operator=(const ExposedObject&) = delete;
};

typedef std::shared_ptr<ExposedObject> ObjectPtr;

template <typename TO, ObjectType TK>
class ExposedObjectT : public ExposedObject {
 public:
  static constexpr ObjectType kType = TK;

  template <typename... TArgs>
  static std::shared_ptr<TO> Create(TArgs&&... args) {
    return std::make_shared<TO>(std::forward<TArgs>(args)...);
  }

  virtual ObjectType Type() const override { return TK; };
};

class ObjectRegister {
 public:
  template <typename TO = ExposedObject>
  static std::shared_ptr<TO> Get(ObjectHandle handle) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto obj = objects_.find(handle);

    if (obj == objects_.end()) {
      throw Error(YOGI_ERR_INVALID_HANDLE);
    }

    YOGI_ASSERT(obj->second->Handle() == handle);
    return obj->second->Cast<TO>();
  }

  static ObjectHandle Register(ObjectPtr obj);
  static void Destroy(ObjectHandle handle);
  static void Clear();

 private:
  typedef std::unordered_map<ObjectHandle, ObjectPtr> ObjectsMap;

  static std::mutex mutex_;
  static ObjectsMap objects_;
};

}  // namespace api
