#include "stdafx.h"

#include <yogi_core.h>

TEST(VersionTest, Macros) {
	std::ostringstream ss;
	ss << YOGI_VERSION_MAJOR << '.' << YOGI_VERSION_MINOR << '.' << YOGI_VERSION_PATCH;

	if (strlen(YOGI_VERSION_SUFFIX)) {
		ss << '-' << YOGI_VERSION_SUFFIX;
	}

	auto version = ss.str();
	EXPECT_EQ(version, YOGI_VERSION);
}

TEST(VersionTest, GetVersion) {
  EXPECT_STREQ(YOGI_VERSION, YOGI_GetVersion());
}
