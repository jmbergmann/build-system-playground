#include "../common.h"

TEST(ObjectFormatTest, FormatObject) {
  void* context;
  int res = YOGI_ContextCreate(&context);
  EXPECT_EQ(res, YOGI_OK);

  char str[32];
  res = YOGI_FormatObject(context, str, sizeof(str), nullptr, nullptr);
  EXPECT_EQ(res, YOGI_OK);
  std::string s1 = str;
  EXPECT_NE(s1.find("Context"), std::string::npos);
  EXPECT_NE(s1.find("["), std::string::npos);
  EXPECT_NE(s1.find("]"), std::string::npos);
  EXPECT_EQ(s1.find("[]"), std::string::npos);

  res = YOGI_FormatObject(context, str, sizeof(str), "$T$x$X", nullptr);
  EXPECT_EQ(res, YOGI_OK);
  std::string s2 = str;
  EXPECT_EQ(s2.find("Context"), 0);
  EXPECT_GT(s2.size(), sizeof("Context") + 2);

  res = YOGI_FormatObject(context, str, sizeof(str), "$T$X$x", nullptr);
  EXPECT_EQ(res, YOGI_OK);
  std::string s3 = str;
  EXPECT_NE(s3, s2);

  res = YOGI_FormatObject(nullptr, str, sizeof(str), nullptr, "abc");
  EXPECT_EQ(res, YOGI_OK);
  std::string s4 = str;
  EXPECT_EQ(s4, "abc");

  res = YOGI_FormatObject(nullptr, str, sizeof(str), "$T $x", "abc");
  EXPECT_EQ(res, YOGI_OK);
  std::string s5 = str;
  EXPECT_EQ(s5, "abc");
}
