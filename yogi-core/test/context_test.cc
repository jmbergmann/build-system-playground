#include <gtest/gtest.h>
#include <yogi_core.h>

class ContextTest : public ::testing::Test {
 protected:
  virtual void SetUp() override {
    context_ = nullptr;
    ASSERT_EQ(YOGI_CreateContext(&context_), YOGI_OK);
    ASSERT_NE(context_, nullptr);
  }

  virtual void TearDown() override { ASSERT_EQ(YOGI_DestroyAll(), YOGI_OK); }

  void* context_;
};

TEST_F(ContextTest, CreateAndDestroy) {
  // Context gets created in SetUp()
  EXPECT_EQ(YOGI_Destroy(context_), YOGI_OK);
}
