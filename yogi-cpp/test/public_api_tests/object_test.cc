#include "../common.h"

#include <regex>

class ObjectTest : public ::testing::Test {
 protected:
  yogi::Context object_;
};

TEST_F(ObjectTest, GetObjectTypeName) {
  EXPECT_EQ(object_.GetObjectTypeName(), "Context");
}

TEST_F(ObjectTest, ToString) {
  CheckStringMatches(object_.ToString(), std::string("^") +
                                             object_.GetObjectTypeName() +
                                             " \\[[1-9a-f][0-9a-f]+\\]$");
}
