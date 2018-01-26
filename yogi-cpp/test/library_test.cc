#include <gtest/gtest.h>
#include <yogi.h>

TEST(LibraryTest, GetVersion) {
  EXPECT_EQ(yogi::GetVersion(), YOGI_GetVersion());
}
