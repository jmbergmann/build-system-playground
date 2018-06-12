#include "common.h"

TEST(TimeTest, GetCurrentTime) {
  long long time_a = 0;
	int res = YOGI_GetCurrentTime(&time_a);
  EXPECT_EQ(res, YOGI_OK);
  EXPECT_GT(time_a, 0);

  long long time_b = 0;
	res = YOGI_GetCurrentTime(&time_b);
  EXPECT_EQ(res, YOGI_OK);
  EXPECT_GT(time_b, time_a);
}
