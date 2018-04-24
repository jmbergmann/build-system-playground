#include "branch.h"

namespace objects {

Branch::Branch(ContextPtr context, std::string name, std::string description,
               std::string netname, std::string password, std::string path,
               std::string interface, int advport,
               std::chrono::milliseconds advint)
    : context_(context) {}

}  // namespace objects
