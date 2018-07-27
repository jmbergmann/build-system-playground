#include <gtest/gtest.h>
#include <yogi.h>

TEST(YogiHeaderTest, CoreHeaderNotIncluded) {
  bool core_header_included = false;

#ifdef YOGI_HDR_VERSION
  core_header_included = true;
#endif

  EXPECT_FALSE(core_header_included);
}
