#include "../common.h"

#include <yogi_core.h>

class TimerTest : public ::testing::Test {
 protected:
  yogi::ContextPtr context_ = yogi::Context::Create();
  yogi::TimerPtr timer_ = yogi::Timer::Create(context_);
};

TEST_F(TimerTest, Start) {
  bool called = false;
  timer_->Start(yogi::Duration::FromMilliseconds(1), [&](auto& res) {
    EXPECT_NO_THROW(dynamic_cast<const yogi::Success&>(res));
    EXPECT_EQ(res.GetErrorCode(), yogi::ErrorCode::kOk);
    called = true;
  });

  while (!called) {
    context_->RunOne();
  }

  EXPECT_TRUE(called);
}

TEST_F(TimerTest, Cancel) {
  EXPECT_FALSE(timer_->Cancel());

  bool called = false;
  timer_->Start(yogi::Duration::kInfinity, [&](auto& res) {
    EXPECT_NO_THROW(dynamic_cast<const yogi::Failure&>(res));
    EXPECT_EQ(res.GetErrorCode(), yogi::ErrorCode::kCanceled);
    called = true;
  });

  EXPECT_TRUE(timer_->Cancel());

  while (!called) {
    context_->RunOne();
  }

  EXPECT_TRUE(called);
}
