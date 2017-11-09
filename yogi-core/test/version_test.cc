#include "stdafx.h"

#include <yogi_core.h>

TEST(VersionTest, GetVersion) {
  EXPECT_STREQ("0.0.3-alpha", YOGI_GetVersion());
}
