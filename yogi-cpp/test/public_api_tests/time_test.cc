#include "../common.h"

TEST(TimeTest, GetCurrentTime) {
  auto a = yogi::GetCurrentTime();
  auto b = yogi::GetCurrentTime();
  EXPECT_LE(a, b);
}
