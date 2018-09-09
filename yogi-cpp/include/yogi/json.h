#ifndef YOGI_JSON_H
#define YOGI_JSON_H

#ifdef NLOHMANN_JSON_VERSION_MAJOR
#if NLOHMANN_JSON_VERSION_MAJOR != 3
#error "Incompatible version of Niels Lohmann's JSON library has been included"
#endif
#else
#include "3rd_party/nlohmann/json.hpp"
#endif

namespace yogi {

using Json = ::nlohmann::json;

}  // namespace yogi

#endif  // YOGI_JSON_H
