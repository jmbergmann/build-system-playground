#pragma once

#include "../config.h"
#include "../api/object.h"

namespace objects {

class Context : public api::ExposedObjectT<Context, api::ObjectType::kContext> {
};

typedef std::shared_ptr<Context> ContextPtr;

}  // namespace objects
