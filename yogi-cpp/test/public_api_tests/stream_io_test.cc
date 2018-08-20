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
  Check(*yogi::Context::Create());
}

TEST(StreamIoTest, Verbosity) {
  Check(yogi::Verbosity::kWarning);
}

TEST(StreamIoTest, Stream) {
  Check(yogi::Stream::kStderr);
}

TEST(StreamIoTest, BranchEvents) {
  Check(yogi::BranchEvents::kBranchDiscovered |
        yogi::BranchEvents::kBranchQueried);
}

TEST(StreamIoTest, Duration) {
  Check(yogi::Duration());
}

TEST(StreamIoTest, Timestamp) {
  Check(yogi::Timestamp::Now());
}

TEST(StreamIoTest, BranchInfo) {
  auto context = yogi::Context::Create();
  auto branch = yogi::Branch::Create(context);
  Check(branch->GetInfo());
}

TEST(StreamIoTest, BranchEventInfo) {
  Check(yogi::BranchEventInfo(yogi::Uuid{}, "{}"));
}

TEST(StreamIoTest, Signals) {
  Check(yogi::Signals::kInt | yogi::Signals::kUsr3);
}

TEST(StreamIoTest, ConfigurationFlags) {
  Check(yogi::ConfigurationFlags::kDisableVariables);
}

TEST(StreamIoTest, CommandLineOptions) {
  Check(yogi::CommandLineOptions::kBranchAll |
        yogi::CommandLineOptions::kLogging);
}
