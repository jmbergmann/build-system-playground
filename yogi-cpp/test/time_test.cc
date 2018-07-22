#include "common.h"

#include <chrono>
using namespace std::chrono_literals;

TEST(TimeTest, GetCurrentTime) {
  auto t = yogi::GetCurrentTime();
  auto now = std::chrono::system_clock::now();
  EXPECT_LT((now - t) + 1s, 2s);
}
