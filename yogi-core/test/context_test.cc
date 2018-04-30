#include <gtest/gtest.h>
#include <yogi_core.h>
#include <atomic>
#include <thread>

using namespace std::chrono_literals;

const auto kTimingMargin = 50ms;

class ContextTest : public ::testing::Test {
 protected:
  virtual void SetUp() override {
    context_ = nullptr;
    int res = YOGI_ContextCreate(&context_);
    ASSERT_EQ(res, YOGI_OK);
    ASSERT_NE(context_, nullptr);
  }

  virtual void TearDown() override {
    int res = YOGI_DestroyAll();
    ASSERT_EQ(res, YOGI_OK);
  }

  void* context_;
};

TEST_F(ContextTest, CreateAndDestroy) {
  // Context gets created in SetUp()
  int res = YOGI_Destroy(context_);
  EXPECT_EQ(res, YOGI_OK);
}

TEST_F(ContextTest, Poll) {
  int res = YOGI_ContextPoll(context_, nullptr);
  EXPECT_EQ(res, YOGI_OK);

  int cnt = -1;
  res = YOGI_ContextPoll(context_, &cnt);
  EXPECT_EQ(res, YOGI_OK);
  EXPECT_EQ(cnt, 0);

  res = YOGI_ContextPost(context_, [](void*) {}, nullptr);
  EXPECT_EQ(res, YOGI_OK);
  res = YOGI_ContextPost(context_, [](void*) {}, nullptr);
  EXPECT_EQ(res, YOGI_OK);

  res = YOGI_ContextPoll(context_, &cnt);
  EXPECT_EQ(res, YOGI_OK);
  EXPECT_EQ(cnt, 2);
}

TEST_F(ContextTest, PollOne) {
  int res = YOGI_ContextPollOne(context_, nullptr);
  EXPECT_EQ(res, YOGI_OK);

  int cnt = -1;
  res = YOGI_ContextPollOne(context_, &cnt);
  EXPECT_EQ(res, YOGI_OK);
  EXPECT_EQ(cnt, 0);

  YOGI_ContextPost(context_, [](void*) {}, nullptr);
  YOGI_ContextPost(context_, [](void*) {}, nullptr);

  res = YOGI_ContextPollOne(context_, &cnt);
  EXPECT_EQ(res, YOGI_OK);
  EXPECT_EQ(cnt, 1);
}

TEST_F(ContextTest, Run) {
  int res;
  std::thread th([&] {
    res = YOGI_ContextRun(context_, nullptr, -1);
  });
  YOGI_ContextWaitForRunning(context_, -1);

  YOGI_ContextStop(context_);
  th.join();
  EXPECT_EQ(res, YOGI_OK);

  int count = -1;
  th = std::thread([&] {
    res = YOGI_ContextRun(context_, &count, -1);
  });
  YOGI_ContextWaitForRunning(context_, -1);

  std::atomic<int> n(0);
  YOGI_ContextPost(context_, [](void* n) { ++*static_cast<int*>(n); }, &n);
  YOGI_ContextPost(context_, [](void* n) { ++*static_cast<int*>(n); }, &n);

  while (n < 2)
    ;

  YOGI_ContextStop(context_);
  th.join();
  EXPECT_EQ(res, YOGI_OK);
  EXPECT_EQ(count, 2);
  EXPECT_EQ(n, 2);
}

TEST_F(ContextTest, RunOne) {
  int res;
  std::thread th([&] {
    res = YOGI_ContextRunOne(context_, nullptr, -1);
  });
  YOGI_ContextWaitForRunning(context_, -1);

  YOGI_ContextStop(context_);
  th.join();
  EXPECT_EQ(res, YOGI_OK);

  int count = -1;
  th = std::thread([&] {
    res = YOGI_ContextRunOne(context_, &count, -1);
  });
  YOGI_ContextWaitForRunning(context_, -1);

  std::atomic<int> n(0);
  YOGI_ContextPost(context_, [](void* n) { ++*static_cast<int*>(n); }, &n);
  YOGI_ContextPost(context_, [](void* n) { ++*static_cast<int*>(n); }, &n);

  while (n < 1)
    ;

  YOGI_ContextStop(context_);
  th.join();
  EXPECT_EQ(res, YOGI_OK);
  EXPECT_EQ(count, 1);
  EXPECT_EQ(n, 1);
}

TEST_F(ContextTest, RunFor) {
  int res = YOGI_ContextRun(context_, nullptr, 0);
  EXPECT_EQ(res, YOGI_OK);

  int n = 0;
  YOGI_ContextPost(context_, [](void* n) { ++*static_cast<int*>(n); }, &n);
  YOGI_ContextPost(context_, [](void* n) { ++*static_cast<int*>(n); }, &n);

  int count = -1;
  auto start_time = std::chrono::steady_clock::now();
  res = YOGI_ContextRun(context_, &count, 5000000);
  auto dur = std::chrono::steady_clock::now() - start_time;

  EXPECT_EQ(res, YOGI_OK);
  EXPECT_GE(dur, 5ms);
  EXPECT_LT(dur, 5ms + kTimingMargin);
  EXPECT_EQ(count, 2);
  EXPECT_EQ(n, 2);
}

TEST_F(ContextTest, RunOneFor) {
  int res = YOGI_ContextRunOne(context_, nullptr, 0);
  EXPECT_EQ(res, YOGI_OK);

  int n = 0;
  YOGI_ContextPost(context_, [](void* n) { ++*static_cast<int*>(n); }, &n);

  int count = -1;
  auto start_time = std::chrono::steady_clock::now();
  res = YOGI_ContextRunOne(context_, &count, 5000000);
  auto dur = std::chrono::steady_clock::now() - start_time;

  EXPECT_EQ(res, YOGI_OK);
  EXPECT_LT(dur, 5ms);
  EXPECT_EQ(count, 1);
  EXPECT_EQ(n, 1);

  start_time = std::chrono::steady_clock::now();
  res = YOGI_ContextRunOne(context_, &count, 5000000);
  dur = std::chrono::steady_clock::now() - start_time;

  EXPECT_EQ(res, YOGI_OK);
  EXPECT_GE(dur, 5ms);
  EXPECT_LT(dur, 5ms + kTimingMargin);
  EXPECT_EQ(count, 0);
  EXPECT_EQ(n, 1);
}

TEST_F(ContextTest, RunInBackground) {
  std::atomic<int> n(0);
  YOGI_ContextPost(context_, [](void* n) { ++*static_cast<int*>(n); }, &n);

  int res = YOGI_ContextRunInBackground(context_);
  EXPECT_EQ(res, YOGI_OK);

  YOGI_ContextPost(context_, [](void* n) { ++*static_cast<int*>(n); }, &n);

  while (n != 2)
    ;
}

TEST_F(ContextTest, WaitForStopped) {
  YOGI_ContextRunInBackground(context_);

  int res = YOGI_ContextWaitForStopped(context_, 0);
  EXPECT_EQ(res, YOGI_ERR_TIMEOUT);

  auto start_time = std::chrono::steady_clock::now();
  res = YOGI_ContextWaitForStopped(context_, 1000000);
  auto dur = std::chrono::steady_clock::now() - start_time;
  EXPECT_EQ(res, YOGI_ERR_TIMEOUT);
  EXPECT_GE(dur, 1ms);
  EXPECT_LT(dur, 1ms + kTimingMargin);

  YOGI_ContextStop(context_);
  res = YOGI_ContextWaitForStopped(context_, 1000000000);
  EXPECT_EQ(res, YOGI_OK);
}

TEST_F(ContextTest, Post) {
  int res = YOGI_ContextPost(context_, [](void*) {}, nullptr);
  EXPECT_EQ(res, YOGI_OK);
}
