#include <gtest/gtest.h>
#include <yogi_core.h>

TEST(VersionTest, Macros) {
	std::ostringstream ss;
	ss << YOGI_HDR_VERSION_MAJOR << '.' << YOGI_HDR_VERSION_MINOR << '.' << YOGI_HDR_VERSION_PATCH;

	auto version = ss.str();
	EXPECT_EQ(version, YOGI_HDR_VERSION);
}

TEST(VersionTest, GetVersion) {
  EXPECT_STREQ(YOGI_HDR_VERSION, YOGI_GetVersion());
}
