#include <gtest/gtest.h>
#include <yogi.h>

TEST(VersionTest, GetVersion) {
  EXPECT_STREQ(YOGI_VERSION, YOGI_GetVersion());
}
