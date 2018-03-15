#include <gtest/gtest.h>
#include <yogi.h>

TEST(VersionTest, GetVersion) {
  EXPECT_EQ(yogi::GetVersion(), YOGI_HDR_VERSION);
}
