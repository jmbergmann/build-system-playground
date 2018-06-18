#include "../common.h"

class SignalSetTest : public Test {
 protected:
  virtual void SetUp() override { context_ = CreateContext(); }

  void* CreateSignalSet(int signals) {
    void* sigset = nullptr;
    int res = YOGI_SignalSetCreate(&sigset, context_, signals);
    EXPECT_EQ(res, YOGI_OK);
    EXPECT_NE(sigset, nullptr);
    return sigset;
  }

  void* context_;
};

TEST_F(SignalSetTest, RaiseSignalWithoutListeners) {
  void* sigset = CreateSignalSet(YOGI_SIG_INT);

  int cnt = -1;
  int res = YOGI_RaiseSignal(YOGI_SIG_TERM, nullptr, &cnt);
  EXPECT_EQ(res, YOGI_OK);
  EXPECT_EQ(cnt, 0);
}

TEST_F(SignalSetTest, Await) {
  void* sigset_1 = CreateSignalSet(YOGI_SIG_INT);
  void* sigset_2 = CreateSignalSet(YOGI_SIG_INT | YOGI_SIG_USR1);
  void* sigset_3 = CreateSignalSet(YOGI_SIG_USR2);

  int remcnt_1 = -1;
  int res = YOGI_SignalSetAwait(
      sigset_1,
      [](int res, int sig, void* evarg, int remcnt, void* userarg) {
        EXPECT_EQ(res, YOGI_OK);
        EXPECT_EQ(sig, YOGI_SIG_INT);
        EXPECT_EQ(*static_cast<int*>(evarg), 123);
        *static_cast<int*>(userarg) = remcnt;
      },
      &remcnt_1);
  EXPECT_EQ(res, YOGI_OK);

  int remcnt_2 = -1;
  res = YOGI_SignalSetAwait(
      sigset_2,
      [](int res, int sig, void* evarg, int remcnt, void* userarg) {
        EXPECT_EQ(res, YOGI_OK);
        EXPECT_EQ(sig, YOGI_SIG_INT);
        EXPECT_EQ(*static_cast<int*>(evarg), 123);
        *static_cast<int*>(userarg) = remcnt;
      },
      &remcnt_2);
  EXPECT_EQ(res, YOGI_OK);

  int remcnt_3 = -1;
  res = YOGI_SignalSetAwait(
      sigset_3,
      [](int res, int sig, void* evarg, int remcnt, void* userarg) {
        EXPECT_EQ(res, YOGI_ERR_CANCELED);
        *static_cast<int*>(userarg) = remcnt;
      },
      &remcnt_3);
  EXPECT_EQ(res, YOGI_OK);

  int evarg = 123;
  int cnt = -1;
  res = YOGI_RaiseSignal(YOGI_SIG_INT, &evarg, &cnt);
  EXPECT_EQ(res, YOGI_OK);
  EXPECT_EQ(cnt, 2);

  YOGI_ContextRunOne(context_, nullptr, -1);
  YOGI_ContextRunOne(context_, nullptr, -1);
  EXPECT_TRUE(remcnt_1 == 0 || remcnt_1 == 1);
  EXPECT_TRUE(remcnt_2 == 0 || remcnt_2 == 1);
  EXPECT_TRUE(remcnt_3 == 0 || remcnt_3 == -1);
  EXPECT_EQ(remcnt_1 + remcnt_2, 1);

  int n = -1;
  YOGI_ContextPollOne(context_, &n);
  EXPECT_EQ(n, 0);
}

TEST_F(SignalSetTest, Queueing) {
  void* sigset = CreateSignalSet(YOGI_SIG_INT);

  int evarg_1 = 123;
  int cnt = -1;
  int res = YOGI_RaiseSignal(YOGI_SIG_INT, &evarg_1, &cnt);
  EXPECT_EQ(res, YOGI_OK);
  EXPECT_EQ(cnt, 1);

  int evarg_2 = 456;
  cnt = -1;
  res = YOGI_RaiseSignal(YOGI_SIG_TERM, &evarg_2, &cnt);
  EXPECT_EQ(res, YOGI_OK);
  EXPECT_EQ(cnt, 0);

  int evarg_3 = 789;
  cnt = -1;
  res = YOGI_RaiseSignal(YOGI_SIG_INT, &evarg_3, &cnt);
  EXPECT_EQ(res, YOGI_OK);
  EXPECT_EQ(cnt, 1);

  int remcnt = -1;
  res = YOGI_SignalSetAwait(
      sigset,
      [](int res, int sig, void* evarg, int remcnt, void* userarg) {
        EXPECT_EQ(res, YOGI_OK);
        EXPECT_EQ(sig, YOGI_SIG_INT);
        EXPECT_EQ(*static_cast<int*>(evarg), 123);
        *static_cast<int*>(userarg) = remcnt;
      },
      &remcnt);
  EXPECT_EQ(res, YOGI_OK);

  YOGI_ContextRunOne(context_, nullptr, -1);
  EXPECT_EQ(remcnt, 0);

  remcnt = -1;
  res = YOGI_SignalSetAwait(
      sigset,
      [](int res, int sig, void* evarg, int remcnt, void* userarg) {
        EXPECT_EQ(res, YOGI_OK);
        EXPECT_EQ(sig, YOGI_SIG_INT);
        EXPECT_EQ(*static_cast<int*>(evarg), 789);
        *static_cast<int*>(userarg) = remcnt;
      },
      &remcnt);
  EXPECT_EQ(res, YOGI_OK);

  YOGI_ContextRunOne(context_, nullptr, -1);
  EXPECT_EQ(remcnt, 0);

  int n = -1;
  YOGI_ContextPollOne(context_, &n);
  EXPECT_EQ(n, 0);
}

TEST_F(SignalSetTest, CancelAwait) {
  void* sigset = CreateSignalSet(YOGI_SIG_INT);

  bool called = false;
  YOGI_SignalSetAwait(
      sigset,
      [](int res, int sig, void* evarg, int remcnt, void* userarg) {
        EXPECT_EQ(res, YOGI_ERR_CANCELED);
        EXPECT_EQ(sig, YOGI_SIG_NONE);
        EXPECT_EQ(evarg, nullptr);
        EXPECT_EQ(remcnt, 0);
        *static_cast<bool*>(userarg) = true;
      },
      &called);

  int res = YOGI_SignalSetCancelAwait(sigset);
  EXPECT_EQ(res, YOGI_OK);

  YOGI_ContextRunOne(context_, nullptr, -1);
  EXPECT_TRUE(called);
}
