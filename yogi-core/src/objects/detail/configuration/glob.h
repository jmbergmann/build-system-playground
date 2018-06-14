#pragma once

#include "../../../config.h"

#include <vector>
#include <string>

namespace objects {
namespace detail {

std::vector<std::string> Glob(const std::vector<std::string>& patterns);

}  // namespace detail
}  // namespace objects
