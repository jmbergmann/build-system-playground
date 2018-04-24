#pragma once

#include "../config.h"
#include "context.h"

#include <chrono>

namespace objects {

class Branch : public api::ExposedObjectT<Branch, api::ObjectType::kBranch> {
 public:
  Branch(ContextPtr context, std::string name, std::string description,
         std::string netname, std::string password, std::string path,
         std::string interface, int advport, std::chrono::milliseconds advint);

 private:
  const ContextPtr context_;
};

typedef std::shared_ptr<Branch> BranchPtr;

}  // namespace objects
