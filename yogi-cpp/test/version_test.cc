#include "common.h"
#include <yogi_core.h>

TEST(LibraryTest, GetVersion) {
  EXPECT_EQ(yogi::GetVersion(), YOGI_HDR_VERSION);
}
