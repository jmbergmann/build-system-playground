#include "../common.h"

TEST(IoTest, ToString) {
  EXPECT_FALSE(yogi::ToString(yogi::ErrorCode::kBadAlloc).empty());
}
