#include <gtest/gtest.h>
#include <yogi_core.h>
#include "../src/errors.h"

TEST(ErrorsTest, GetErrorString) {
  EXPECT_STREQ(GetErrorString(-3), YOGI_GetErrorString(-3));
  EXPECT_STRNE(GetErrorString(-2), YOGI_GetErrorString(-3));
  EXPECT_STREQ(YOGI_GetErrorString(0), YOGI_GetErrorString(1));
}

TEST(ErrorsTest, DescriptionForEachError) {
  auto last_error = YOGI_ERR_CANCELED;
  auto inv_err_str = YOGI_GetErrorString(last_error - 1);

  for (int i = 0; i >= last_error; --i) {
    EXPECT_STRNE(YOGI_GetErrorString(i), inv_err_str) << "Error code: " << i;
  }

  for (int i = last_error - 1; i < last_error - 10; --i) {
    EXPECT_STREQ(YOGI_GetErrorString(i), inv_err_str)  << "Error code: " << i;
  }
}
