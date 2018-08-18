#include "../common.h"

#include <yogi_core.h>

class BranchTest : public ::testing::Test {
 protected:
  yogi::ContextPtr context_ = yogi::Context::Create();
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
  auto branch = yogi::Branch::Create(
      context_, "My Branch", "Stuff", "My Network", "Password", "/some/path",
      "239.255.0.1", 12345, yogi::Duration::FromSeconds(7),
      yogi::Duration::kInfinity);

  auto info = branch->GetInfo();
  EXPECT_NE(info.GetUuid(), yogi::Uuid{});
  EXPECT_EQ(info.GetName(), "My Branch");
  EXPECT_EQ(info.GetDescription(), "Stuff");
  EXPECT_EQ(info.GetNetName(), "My Network");
  EXPECT_EQ(info.GetPath(), "/some/path");
  EXPECT_FALSE(info.GetHostname().empty());
  EXPECT_GT(info.GetPid(), 0);
  EXPECT_EQ(info.GetAdvertisingAddress(), "239.255.0.1");
  EXPECT_EQ(info.GetAdvertisingPort(), 12345);
  EXPECT_EQ(info.GetAdvertisingInterval(), yogi::Duration::FromSeconds(7));
  EXPECT_FALSE(info.GetTcpServerAddress().empty());
  EXPECT_GT(info.GetTcpServerPort(), 0);
  EXPECT_LT(info.GetStartTime(), yogi::GetCurrentTime());
  EXPECT_EQ(info.GetTimeout(), yogi::Duration::kInfinity);

  EXPECT_EQ(branch->GetUuid(), info.GetUuid());
  EXPECT_EQ(branch->GetName(), info.GetName());
  EXPECT_EQ(branch->GetDescription(), info.GetDescription());
  EXPECT_EQ(branch->GetNetName(), info.GetNetName());
  EXPECT_EQ(branch->GetPath(), info.GetPath());
  EXPECT_EQ(branch->GetHostname(), info.GetHostname());
  EXPECT_EQ(branch->GetPid(), info.GetPid());
  EXPECT_EQ(branch->GetAdvertisingAddress(), info.GetAdvertisingAddress());
  EXPECT_EQ(branch->GetAdvertisingPort(), info.GetAdvertisingPort());
  EXPECT_EQ(branch->GetAdvertisingInterval(), info.GetAdvertisingInterval());
  EXPECT_EQ(branch->GetTcpServerAddress(), info.GetTcpServerAddress());
  EXPECT_EQ(branch->GetTcpServerPort(), info.GetTcpServerPort());
  EXPECT_EQ(branch->GetStartTime(), info.GetStartTime());
  EXPECT_EQ(branch->GetTimeout(), info.GetTimeout());
}

TEST_F(BranchTest, GetConnectedBranches) {
  auto branch = yogi::Branch::Create(context_, "My Branch");
  auto branch_a = yogi::Branch::Create(context_, "A");
  auto branch_b = yogi::Branch::Create(context_, "B");

  while (!branch->GetConnectedBranches().count(branch_a->GetUuid()) ||
         !branch->GetConnectedBranches().count(branch_b->GetUuid())) {
    context_->RunOne();
  }

  auto branches = branch->GetConnectedBranches();

  ASSERT_EQ(branches.count(branch_a->GetUuid()), 1);
  EXPECT_EQ(branches.at(branch_a->GetUuid()).GetName(), branch_a->GetName());

  ASSERT_EQ(branches.count(branch_b->GetUuid()), 1);
  EXPECT_EQ(branches.at(branch_b->GetUuid()).GetName(), branch_b->GetName());
}

TEST_F(BranchTest, AwaitEvent) {
  auto branch = yogi::Branch::Create(context_, "My Branch");
  auto branch_a = yogi::Branch::Create(context_, "A");

  auto events =
      yogi::BranchEvents::kBranchQueried | yogi::BranchEvents::kConnectionLost;
  bool called = false;
  branch->AwaitEvent(events, [&](auto& res, auto event, auto& evres,
                                 auto& info) {
    EXPECT_NO_THROW(dynamic_cast<const yogi::Success&>(res));
    EXPECT_EQ(res.GetErrorCode(), yogi::ErrorCode::kOk);
    EXPECT_EQ(event, yogi::BranchEvents::kBranchQueried);
    EXPECT_NO_THROW(dynamic_cast<const yogi::Success&>(evres));
    EXPECT_EQ(evres.GetErrorCode(), yogi::ErrorCode::kOk);
    EXPECT_NO_THROW(dynamic_cast<const yogi::BranchQueriedEventInfo&>(info));
    EXPECT_EQ(
        static_cast<const yogi::BranchQueriedEventInfo&>(info).GetStartTime(),
        branch_a->GetStartTime());
    EXPECT_EQ(
        static_cast<const yogi::BranchQueriedEventInfo&>(info).GetTimeout(),
        branch_a->GetTimeout());
    called = true;
  });

  while (!called) {
    context_->RunOne();
  }

  EXPECT_TRUE(called);
}

TEST_F(BranchTest, CancelAwaitEvent) {
  auto branch = yogi::Branch::Create(context_, "My Branch");

  bool called = false;
  branch->AwaitEvent(yogi::BranchEvents::kAll, [&](auto& res, auto event,
                                                   auto& evres, auto&) {
    EXPECT_NO_THROW(dynamic_cast<const yogi::Failure&>(res));
    EXPECT_EQ(res.GetErrorCode(), yogi::ErrorCode::kCanceled);
    EXPECT_EQ(event, yogi::BranchEvents::kNone);
    EXPECT_NO_THROW(dynamic_cast<const yogi::Success&>(evres));
    EXPECT_EQ(evres.GetErrorCode(), yogi::ErrorCode::kOk);
    called = true;
  });

  branch->CancelAwaitEvent();
  context_->Poll();
  EXPECT_TRUE(called);
}
