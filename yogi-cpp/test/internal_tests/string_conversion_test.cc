#include "../common.h"

using namespace yogi::internal;

TEST(ConversionTest, ToCoreString) {
  const char* str = "";
  EXPECT_EQ(ToCoreString(str), str);

  std::string s = "abc";
  EXPECT_EQ(ToCoreString(s), s.c_str());
}
