#include <gtest/gtest.h>
#include <yogi.h>
#include <yogi_core.h>

TEST(LibraryTest, GetVersion) {
  EXPECT_EQ(yogi::GetVersion(), YOGI_HDR_VERSION);
}
