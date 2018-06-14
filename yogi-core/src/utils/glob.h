#pragma once

#include "../config.h"

#include <vector>
#include <string>

namespace utils {

std::vector<std::string> Glob(const std::vector<std::string>& patterns);

}  // namespace utils
