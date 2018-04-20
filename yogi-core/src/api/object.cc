#include "object.h"

#include <limits>

namespace api {

std::mutex ObjectRegister::mutex_;
ObjectRegister::ObjectsMap ObjectRegister::objects_;

ObjectHandle ObjectRegister::Register(ObjectPtr obj) {
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

  auto old_size = std::numeric_limits<std::size_t>::max();
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
