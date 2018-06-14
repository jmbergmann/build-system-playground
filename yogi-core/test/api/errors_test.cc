#include "../common.h"

static constexpr int kLastError = YOGI_ERR_HELP_REQUESTED;

TEST(ErrorsTest, GetErrorString) {
  EXPECT_STRNE(YOGI_GetErrorString(-2), YOGI_GetErrorString(-1));
  EXPECT_STREQ(YOGI_GetErrorString(0), YOGI_GetErrorString(1));
}

TEST(ErrorsTest, DescriptionForEachError) {
  auto inv_err_str = YOGI_GetErrorString(kLastError - 1);

  for (int i = 0; i >= kLastError; --i) {
    EXPECT_STRNE(YOGI_GetErrorString(i), inv_err_str) << "Error code: " << i;
  }

  for (int i = kLastError - 1; i < kLastError - 10; --i) {
    EXPECT_STREQ(YOGI_GetErrorString(i), inv_err_str)  << "Error code: " << i;
  }
}

TEST(ErrorsTest, NoDuplicates) {
  for (int i = 0; i >= kLastError; --i) {
    for (int j = 0; j > i; --j) {
      EXPECT_STRNE(YOGI_GetErrorString(i), YOGI_GetErrorString(j));
    }
  }
}
