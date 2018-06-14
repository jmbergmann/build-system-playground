#include "../common.h"

#include <signal.h>

class SignalsTest : public Test {
 protected:
  virtual void SetUp() override { context_ = CreateContext(); }

  void* context_;
};

TEST_F(SignalsTest, SigInt) {
  bool called = false;
  int res = YOGI_AwaitSignal(context_, YOGI_SIG_INT,
                             [](int err, int sig, void* userarg) {
                               EXPECT_EQ(err, YOGI_OK);
                               EXPECT_EQ(sig, YOGI_SIG_INT);
                               *static_cast<bool*>(userarg) = true;
                             },
                             &called);
  EXPECT_EQ(res, YOGI_OK);

  raise(SIGINT);

  YOGI_ContextRunOne(context_, nullptr, -1);
  EXPECT_TRUE(called);
}

TEST_F(SignalsTest, SigTerm) {
  bool called = false;
  int res = YOGI_AwaitSignal(context_, YOGI_SIG_TERM,
                             [](int res, int sig, void* userarg) {
                               EXPECT_EQ(res, YOGI_OK);
                               EXPECT_EQ(sig, YOGI_SIG_TERM);
                               *static_cast<bool*>(userarg) = true;
                             },
                             &called);
  EXPECT_EQ(res, YOGI_OK);

  raise(SIGTERM);

  YOGI_ContextRunOne(context_, nullptr, -1);
  EXPECT_TRUE(called);
}

TEST_F(SignalsTest, ReplaceHandler) {
  bool called = false;
  YOGI_AwaitSignal(context_, 0,
                   [](int res, int sig, void* userarg) {
                     EXPECT_EQ(res, YOGI_ERR_CANCELED);
                     EXPECT_EQ(sig, YOGI_SIG_INT | YOGI_SIG_TERM);
                     *static_cast<bool*>(userarg) = true;
                   },
                   &called);

  YOGI_AwaitSignal(context_, YOGI_SIG_INT,
                   [](int res, int sig, void* userarg) {
                     EXPECT_EQ(res, YOGI_OK);
                     EXPECT_EQ(sig, YOGI_SIG_INT);
                     *static_cast<bool*>(userarg) = true;
                   },
                   &called);

  YOGI_ContextRunOne(context_, nullptr, -1);
  EXPECT_TRUE(called);

  called = false;

  raise(SIGINT);

  YOGI_ContextRunOne(context_, nullptr, -1);
  EXPECT_TRUE(called);
}

TEST_F(SignalsTest, CancelAwaitSignal) {
  bool called = false;
  YOGI_AwaitSignal(context_, 0,
                   [](int res, int sig, void* userarg) {
                     EXPECT_EQ(res, YOGI_ERR_CANCELED);
                     EXPECT_EQ(sig, YOGI_SIG_INT | YOGI_SIG_TERM);
                     *static_cast<bool*>(userarg) = true;
                   },
                   &called);

  int res = YOGI_CancelAwaitSignal(context_);
  EXPECT_EQ(res, YOGI_OK);

  YOGI_ContextRunOne(context_, nullptr, -1);
  EXPECT_TRUE(called);
}
