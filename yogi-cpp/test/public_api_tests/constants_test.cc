#include "../common.h"

#include <yogi_core.h>

using namespace std::chrono_literals;

TEST(ConstantsTest, kVersionNumber) {
  EXPECT_EQ(yogi::constants::kVersionNumber, YOGI_HDR_VERSION);
}

TEST(ConstantsTest, kVersionMajor) {
  EXPECT_EQ(yogi::constants::kVersionMajor, YOGI_HDR_VERSION_MAJOR);
}

TEST(ConstantsTest, kVersionMinor) {
  EXPECT_EQ(yogi::constants::kVersionMinor, YOGI_HDR_VERSION_MINOR);
}

TEST(ConstantsTest, kVersionPatch) {
  EXPECT_EQ(yogi::constants::kVersionPatch, YOGI_HDR_VERSION_PATCH);
}

TEST(ConstantsTest, kDefaultAdvAddress) {
  EXPECT_FALSE(yogi::constants::kDefaultAdvAddress.empty());
}

TEST(ConstantsTest, kDefaultAdvPort) {
  EXPECT_GE(yogi::constants::kDefaultAdvPort, 1024);
}

TEST(ConstantsTest, kDefaultAdvInterval) {
  EXPECT_GT(yogi::constants::kDefaultAdvInterval, 500ms);
}

TEST(ConstantsTest, kDefaultConnectionTimeout) {
  EXPECT_GT(yogi::constants::kDefaultConnectionTimeout, 500ms);
}

TEST(ConstantsTest, kDefaultLoggerVerbosity) {
  EXPECT_EQ(yogi::Verbosity::kInfo, yogi::constants::kDefaultLoggerVerbosity);
}

TEST(ConstantsTest, kDefaultLogTimeFormat) {
  EXPECT_FALSE(yogi::constants::kDefaultLogTimeFormat.empty());
}

TEST(ConstantsTest, kDefaultLogFormat) {
  EXPECT_FALSE(yogi::constants::kDefaultLogFormat.empty());
}

TEST(ConstantsTest, kMaxMessageSize) {
  EXPECT_GE(yogi::constants::kMaxMessageSize, 1000);
}

TEST(ConstantsTest, kDefaultTimeFormat) {
  EXPECT_NE(yogi::constants::kDefaultTimeFormat.find(".%3"), std::string::npos);
}

TEST(ConstantsTest, kDefaultInfiniteDurationString) {
  EXPECT_NE(yogi::constants::kDefaultInfiniteDurationString.find("nfinit"),
            std::string::npos);
}

TEST(ConstantsTest, kDefaultDurationFormat) {
  EXPECT_NE(yogi::constants::kDefaultDurationFormat.find(".%3"),
            std::string::npos);
}

TEST(ConstantsTest, kDefaultInvalidHandleString) {
  EXPECT_NE(yogi::constants::kDefaultInvalidHandleString.find("INVALID"),
            std::string::npos);
}

TEST(ConstantsTest, kDefaultObjectFormat) {
  EXPECT_NE(yogi::constants::kDefaultObjectFormat.find("$T"),
            std::string::npos);
}
