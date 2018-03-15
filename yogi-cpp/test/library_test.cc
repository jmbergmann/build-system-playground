#include <gtest/gtest.h>

TEST(LibraryTest, CoreHeaderNotIncluded) {
  bool core_header_included = false;

#ifdef YOGI_HDR_VERSION
  core_header_included = true;
#endif

  EXPECT_FALSE(core_header_included);
}

// This include is here because of the test above
#include <yogi.h>

TEST(LibraryTest, GetVersion) {
  EXPECT_EQ(yogi::GetVersion(), YOGI_HDR_VERSION);
}
