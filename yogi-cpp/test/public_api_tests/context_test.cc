#include "../common.h"

#include <regex>
#include <thread>
#include <chrono>
using namespace std::chrono_literals;

class ContextTest : public ::testing::Test {
 public:
  yogi::Context context_;
};

TEST_F(ContextTest, Poll) {
  EXPECT_EQ(context_.Poll(), 0);
  context_.Post([] {});
  context_.Post([] {});
  EXPECT_EQ(context_.Poll(), 2);
}

TEST_F(ContextTest, PollOne) {
  EXPECT_EQ(context_.PollOne(), 0);
  context_.Post([] {});
  context_.Post([] {});
  EXPECT_EQ(context_.PollOne(), 1);
}

TEST_F(ContextTest, Run) {
  EXPECT_EQ(context_.Run(1ms), 0);
  context_.Post([] {});
  context_.Post([] {});
  EXPECT_EQ(context_.Run(1ms), 2);
}

TEST_F(ContextTest, RunOne) {
  EXPECT_EQ(context_.RunOne(1ms), 0);
  context_.Post([] {});
  context_.Post([] {});
  EXPECT_EQ(context_.RunOne(1ms), 1);
}

TEST_F(ContextTest, RunInBackground) {
  context_.RunInBackground();

  bool called = false;
  context_.Post([&] { called = true; });

  while (!called)
    ;
}

TEST_F(ContextTest, Stop) {
  auto th = std::thread([&] {
    context_.WaitForRunning();
    context_.Stop();
  });

  context_.Run();
  th.join();
}

TEST_F(ContextTest, WaitForRunningAndStopped) {
  context_.WaitForStopped();
  EXPECT_TRUE(context_.WaitForStopped(1ms));
  EXPECT_FALSE(context_.WaitForRunning(1ms));

  context_.RunInBackground();

  context_.WaitForRunning();
  EXPECT_TRUE(context_.WaitForRunning(1ms));
  EXPECT_FALSE(context_.WaitForStopped(1ms));

  context_.Stop();
  context_.WaitForStopped();
}
