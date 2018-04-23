#include <gtest/gtest.h>
#include <yogi_core.h>
#include <chrono>

using namespace std::chrono_literals;

const auto kTimingMargin = 50ms;

class TimerTest : public ::testing::Test {
 protected:
  virtual void SetUp() override {
    context_ = nullptr;
    int res = YOGI_ContextCreate(&context_);
    ASSERT_EQ(res, YOGI_OK);
    ASSERT_NE(context_, nullptr);

    timer_ = nullptr;
    res = YOGI_TimerCreate(&timer_, context_);
    ASSERT_EQ(res, YOGI_OK);
    ASSERT_NE(timer_, nullptr);
  }

  virtual void TearDown() override {
    int res = YOGI_DestroyAll();
    ASSERT_EQ(res, YOGI_OK);
  }

  void* context_;
  void* timer_;
};

TEST_F(TimerTest, Start) {
  int handler_res = 1;
  int res = YOGI_TimerStart(
      timer_, 0, 1000000,  // 1ms timeout
      [](int res, void* handler_res) { *static_cast<int*>(handler_res) = res; },
      &handler_res);
  auto start_time = std::chrono::steady_clock::now();
  EXPECT_EQ(res, YOGI_OK);

  YOGI_ContextRunOneFor(context_, nullptr, 1, 0);
  auto dur = std::chrono::steady_clock::now() - start_time;

  EXPECT_EQ(handler_res, YOGI_OK);
  EXPECT_GE(dur, 1ms);
  EXPECT_LT(dur, 1ms + kTimingMargin);
}

TEST_F(TimerTest, StartWhileRunning) {
  int handler_res = 1;
  int res = YOGI_TimerStart(
      timer_, -1, 0,  // Infinite timeout
      [](int res, void* handler_res) { *static_cast<int*>(handler_res) = res; },
      &handler_res);
  EXPECT_EQ(res, YOGI_OK);

  res = YOGI_TimerStart(
      timer_, 0, 0,  // Immediate timeout
      [](int res, void* handler_res) { *static_cast<int*>(handler_res) = res; },
      &handler_res);
  EXPECT_EQ(res, YOGI_OK);

  YOGI_ContextRunOne(context_, nullptr);
  EXPECT_EQ(handler_res, YOGI_ERR_CANCELED);

  YOGI_ContextRunOne(context_, nullptr);
  EXPECT_EQ(handler_res, YOGI_OK);
}

TEST_F(TimerTest, Cancel) {
  int res = YOGI_TimerCancel(timer_);
  EXPECT_EQ(res, YOGI_ERR_TIMER_EXPIRED);

  int handler_res = 1;
  res = YOGI_TimerStart(
      timer_, -1, 0,  // Infinite timeout
      [](int res, void* handler_res) { *static_cast<int*>(handler_res) = res; },
      &handler_res);

  res = YOGI_TimerCancel(timer_);
  EXPECT_EQ(res, YOGI_OK);

  YOGI_ContextRunOne(context_, nullptr);

  EXPECT_EQ(handler_res, YOGI_ERR_CANCELED);
}
