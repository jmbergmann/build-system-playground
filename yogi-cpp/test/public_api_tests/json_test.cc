#define NLOHMANN_JSON_VERSION_MAJOR 3

namespace nlohmann {

using json = int;

}  // namespace nlohmann

#include <yogi/json.h>
#include <gtest/gtest.h>
#include <type_traits>

TEST(JsonTest, CustomJsonLibrary) {
  EXPECT_TRUE((std::is_same<yogi::json, int>::value));
}
