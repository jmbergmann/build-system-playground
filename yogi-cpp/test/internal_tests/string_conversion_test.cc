#include "../common.h"

using namespace yogi::internal;

TEST(ConversionTest, StringToCoreString) {
  const char* str = "";
  EXPECT_EQ(StringToCoreString(str), str);

  std::string s = "abc";
  EXPECT_EQ(StringToCoreString(s), s.c_str());
}
