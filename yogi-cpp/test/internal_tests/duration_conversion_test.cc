#include "../common.h"

using namespace yogi::internal;
using namespace std::chrono_literals;

TEST(ConversionTest, ToCoreDuration) {
  EXPECT_EQ(ToCoreDuration(123us), 123000);
  EXPECT_EQ(ToCoreDuration(yogi::Duration::kInfinity), -1);
  EXPECT_THROW(ToCoreDuration(-1ms), yogi::FailureException);
}
