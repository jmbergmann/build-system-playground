#include <gtest/gtest.h>
#include <yogi_core.h>

class BranchTest : public ::testing::Test {
 protected:
  virtual void SetUp() override {
    context_ = nullptr;
    ASSERT_EQ(YOGI_CreateContext(&context_), YOGI_OK);
    ASSERT_NE(context_, nullptr);

    branch_ = nullptr;
    ASSERT_EQ(YOGI_CreateBranch(&branch_, context_, nullptr, nullptr, 0, 0),
              YOGI_OK);
    ASSERT_NE(branch_, nullptr);
  }

  virtual void TearDown() override { ASSERT_EQ(YOGI_DestroyAll(), YOGI_OK); }

  void* context_;
  void* branch_;
};

TEST_F(BranchTest, CreateAndDestroy) {
  // Branch gets created in SetUp()
  EXPECT_EQ(YOGI_Destroy(branch_), YOGI_OK);
}
