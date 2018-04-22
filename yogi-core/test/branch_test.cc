#include <gtest/gtest.h>
#include <yogi_core.h>

class BranchTest : public ::testing::Test {
 protected:
  virtual void SetUp() override {
    context_ = nullptr;
    int res = YOGI_ContextCreate(&context_);
    ASSERT_EQ(res, YOGI_OK);
    ASSERT_NE(context_, nullptr);

    branch_ = nullptr;
    res = YOGI_BranchCreate(&branch_, context_, nullptr, nullptr, 0, 0);
    ASSERT_EQ(res, YOGI_OK);
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
