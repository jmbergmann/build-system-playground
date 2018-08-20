#include "object.h"

#include <limits>

namespace api {

const std::string& ExposedObject::TypeName() const {
  switch (Type()) {
    case ObjectType::kDummy: {
      static const std::string s = "Dummy";
      return s;
    }

    case ObjectType::kContext: {
      static const std::string s = "Context";
      return s;
    }

    case ObjectType::kLogger: {
      static const std::string s = "Logger";
      return s;
    }

    case ObjectType::kTimer: {
      static const std::string s = "Timer";
      return s;
    }

    case ObjectType::kBranch: {
      static const std::string s = "Branch";
      return s;
    }

    case ObjectType::kConfiguration: {
      static const std::string s = "Configuration";
      return s;
    }

    case ObjectType::kSignalSet: {
      static const std::string s = "SignalSet";
      return s;
    }

    default: {
      YOGI_NEVER_REACHED;
      static const std::string s;
      return s;
    }
  }
}

std::mutex ObjectRegister::mutex_;
// NOLINTNEXTLINE(cert-err58-cpp)
ObjectRegister::ObjectsMap ObjectRegister::objects_;

ObjectHandle ObjectRegister::Register(const ObjectPtr& obj) {
  std::lock_guard<std::mutex> lock(mutex_);

  auto handle = obj->Handle();
  YOGI_ASSERT(!objects_.count(handle));
  objects_[handle] = obj;

  return handle;
}

void ObjectRegister::Destroy(ObjectHandle handle) {
  ObjectPtr obj;  // Hold it so it gets destroyed AFTER the lock is released

  std::lock_guard<std::mutex> lock(mutex_);
  auto it = objects_.find(handle);

  if (it == objects_.end()) {
    throw Error(YOGI_ERR_INVALID_HANDLE);
  }

  if (!it->second.unique()) {
    throw Error(YOGI_ERR_OBJECT_STILL_USED);
  }

  obj = it->second;
  objects_.erase(it);
}

void ObjectRegister::DestroyAll() {
  ObjectsMap objs;

  {
    std::lock_guard<std::mutex> lock(mutex_);
    std::swap(objects_, objs);
  }

  auto old_size = std::numeric_limits<size_t>::max();
  while (objs.size() < old_size) {
    old_size = objs.size();

    for (auto it = objs.begin(); it != objs.end();) {
      if (it->second.unique()) {
        it = objs.erase(it);
      } else {
        ++it;
      }
    }
  }

  YOGI_ASSERT(objs.empty());
}

}  // namespace api
