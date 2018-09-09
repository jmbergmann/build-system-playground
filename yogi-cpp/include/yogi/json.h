#pragma once

#ifdef NLOHMANN_JSON_VERSION_MAJOR
#if NLOHMANN_JSON_VERSION_MAJOR != 3
#error "Incompatible version of Niels Lohmann's JSON library has been included"
#endif
#else
#include "3rd_party/nlohmann/json.hpp"
#endif

namespace yogi {

using json = ::nlohmann::json;

}  // namespace yogi
