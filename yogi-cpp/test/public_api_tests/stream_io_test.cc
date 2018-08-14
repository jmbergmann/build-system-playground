#include "../common.h"

template <typename T>
void Check(const T& val) {
  std::ostringstream oss;
  oss << val;
  EXPECT_EQ(oss.str(), yogi::ToString(val));
}

TEST(StreamIoTest, ErrorCode) {
  Check(yogi::ErrorCode::kBusy);
}

TEST(StreamIoTest, Result) {
  Check(yogi::Failure(yogi::ErrorCode::kBadAlloc));
}

TEST(StreamIoTest, Object) {
  Check(yogi::Context());
}

TEST(StreamIoTest, Verbosity) {
  Check(yogi::Verbosity::kWarning);
}

TEST(StreamIoTest, Stream) {
  Check(yogi::Stream::kStderr);
}

TEST(StreamIoTest, BranchEvents) {
  Check(yogi::BranchEvents::kBranchDiscovered);
}

TEST(StreamIoTest, Duration) {
  Check(yogi::Duration());
}

TEST(StreamIoTest, Timestamp) {
  Check(yogi::Timestamp::Now());
}