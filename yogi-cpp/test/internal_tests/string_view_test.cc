#include "../common.h"

using namespace yogi::internal;

TEST(StringViewTest, Default) {
  EXPECT_EQ(static_cast<const char*>(StringView()), nullptr);
}

TEST(StringViewTest, ConstCharString) {
  const char* s = "Hello";
  EXPECT_EQ(static_cast<const char*>(StringView(s)), s);
}

TEST(StringViewTest, StdString) {
  std::string s = "Hello";
  EXPECT_EQ(static_cast<const char*>(StringView(s)), s.c_str());
}
