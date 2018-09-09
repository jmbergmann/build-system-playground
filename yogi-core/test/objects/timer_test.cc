#include "../common.h"

class TimerTest : public Test {
 protected:
  virtual void SetUp() override {
    context_ = CreateContext();

    timer_ = nullptr;
    int res = YOGI_TimerCreate(&timer_, context_);
    ASSERT_EQ(res, YOGI_OK);
    ASSERT_NE(timer_, nullptr);
  }

  void* context_;
  void* timer_;
};

TEST_F(TimerTest, Start) {
  int handler_res = 1;
  auto start_time = std::chrono::steady_clock::now();
  int res = YOGI_TimerStart(timer_, 1000000,  // 1ms timeout
                            [](int res_, void* handler_res_) {
                              *static_cast<int*>(handler_res_) = res_;
                            },
                            &handler_res);
  EXPECT_EQ(res, YOGI_OK);

  YOGI_ContextRunOne(context_, nullptr, 1000000000);
  auto dur = std::chrono::steady_clock::now() - start_time;

  EXPECT_EQ(handler_res, YOGI_OK);
  EXPECT_GE(dur, 1ms);
  EXPECT_LT(dur, 1ms + kTimingMargin);
}

TEST_F(TimerTest, StartImmediateTimeout) {
  int handler_res = 1;
  int res = YOGI_TimerStart(timer_, 0,  // Immediate timeout
                            [](int res_, void* handler_res_) {
                              *static_cast<int*>(handler_res_) = res_;
                            },
                            &handler_res);
  EXPECT_EQ(res, YOGI_OK);

  int cnt = -1;
  YOGI_ContextRunOne(context_, &cnt, 1000000000);
  ASSERT_EQ(cnt, 1);
  EXPECT_EQ(handler_res, YOGI_OK);
}

TEST_F(TimerTest, StartWhileRunning) {
  int handler_res = 1;
  int res = YOGI_TimerStart(timer_, -1,  // Infinite timeout
                            [](int res_, void* handler_res_) {
                              *static_cast<int*>(handler_res_) = res_;
                            },
                            &handler_res);
  EXPECT_EQ(res, YOGI_OK);

  res = YOGI_TimerStart(timer_, 0,  // Immediate timeout
                        [](int res_, void* handler_res_) {
                          *static_cast<int*>(handler_res_) = res_;
                        },
                        &handler_res);
  EXPECT_EQ(res, YOGI_OK);

  int cnt = -1;
  YOGI_ContextRunOne(context_, &cnt, 1000000000);
  ASSERT_EQ(cnt, 1);
  EXPECT_EQ(handler_res, YOGI_ERR_CANCELED);

  cnt = -1;
  YOGI_ContextRunOne(context_, &cnt, 1000000000);
  ASSERT_EQ(cnt, 1);
  EXPECT_EQ(handler_res, YOGI_OK);
}

TEST_F(TimerTest, Cancel) {
  int res = YOGI_TimerCancel(timer_);
  EXPECT_EQ(res, YOGI_ERR_TIMER_EXPIRED);

  int handler_res = 1;
  res = YOGI_TimerStart(timer_, -1,  // Infinite timeout
                        [](int res_, void* handler_res_) {
                          *static_cast<int*>(handler_res_) = res_;
                        },
                        &handler_res);

  res = YOGI_TimerCancel(timer_);
  EXPECT_EQ(res, YOGI_OK);

  YOGI_ContextRunOne(context_, nullptr, -1);

  EXPECT_EQ(handler_res, YOGI_ERR_CANCELED);
}

TEST_F(TimerTest, Destruction) {
  int handler_res = 1;
  int res = YOGI_TimerStart(timer_, -1,  // Infinite timeout
                            [](int res_, void* handler_res_) {
                              *static_cast<int*>(handler_res_) = res_;
                            },
                            &handler_res);
  ASSERT_EQ(YOGI_OK, res);

  YOGI_Destroy(timer_);

  YOGI_ContextRunOne(context_, nullptr, -1);
  EXPECT_EQ(handler_res, YOGI_ERR_CANCELED);
}
