#include "common.h"

#include <yogi_core.h>

class BranchTest : public ::testing::Test {
 protected:
  yogi::Context context;
};

TEST_F(BranchTest, BranchEventsEnum) {
  // clang-format off
  CHECK_ENUM_ELEMENT(BranchEvents, kNone,             YOGI_BEV_NONE);
  CHECK_ENUM_ELEMENT(BranchEvents, kBranchDiscovered, YOGI_BEV_BRANCH_DISCOVERED);
  CHECK_ENUM_ELEMENT(BranchEvents, kBranchQueried,    YOGI_BEV_BRANCH_QUERIED);
  CHECK_ENUM_ELEMENT(BranchEvents, kConnectFinished,  YOGI_BEV_CONNECT_FINISHED);
  CHECK_ENUM_ELEMENT(BranchEvents, kConnectionLost,   YOGI_BEV_CONNECTION_LOST);
  CHECK_ENUM_ELEMENT(BranchEvents, kAll,              YOGI_BEV_ALL);
  // clang-format on

  auto events = yogi::BranchEvents::kNone;
  events = events | yogi::BranchEvents::kBranchDiscovered;
  EXPECT_EQ(yogi::ToString(events), "kBranchDiscovered");
  events |= yogi::BranchEvents::kConnectFinished;
  EXPECT_EQ(yogi::ToString(events), "kBranchDiscovered | kConnectFinished");
}

TEST_F(BranchTest, Info) {

}

TEST_F(BranchTest, GetConnectedBranches) {

}

TEST_F(BranchTest, AwaitEvent) {

}

TEST_F(BranchTest, CancelAwaitEvent) {

}
