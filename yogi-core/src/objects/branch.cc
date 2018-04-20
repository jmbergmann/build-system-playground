#include "branch.h"

namespace objects {

Branch::Branch(ContextPtr context, std::string netname, std::string interface,
               int advport, std::chrono::milliseconds advint)
    : context_(context) {}

}  // namespace objects
