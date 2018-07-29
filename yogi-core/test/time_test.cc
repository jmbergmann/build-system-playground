#include "common.h"

TEST(TimeTest, GetCurrentTime) {
  long long time_a = 0;
	int res = YOGI_GetCurrentTime(&time_a);
  EXPECT_EQ(res, YOGI_OK);
  EXPECT_GT(time_a, 0);

  long long time_b = 0;
	res = YOGI_GetCurrentTime(&time_b);
  EXPECT_EQ(res, YOGI_OK);
  EXPECT_GT(time_b, time_a);
}

TEST(TimeTest, FormatTime) {
  long long timestamp = 1234356789123456789LL;
  char str[32];

  int res = YOGI_FormatTime(timestamp, str, sizeof(str), nullptr);
  EXPECT_EQ(res, YOGI_OK);
  std::string s1 = str;
  EXPECT_EQ(s1, "2009-02-11T12:53:09.123Z");

  res = YOGI_FormatTime(timestamp, str, sizeof(str), "%FT%T.%3Z");
  EXPECT_EQ(res, YOGI_OK);
  std::string s2 = str;
  EXPECT_EQ(s1, s2);


  res = YOGI_FormatTime(timestamp, str, sizeof(str), "%Y%m%d%H%M%S%3%6%9");
  EXPECT_EQ(res, YOGI_OK);
  std::string s3 = str;
  EXPECT_EQ(s3, "20090211125309123456789");

  res = YOGI_FormatTime(timestamp, str, 6, "%Y%m%d%H%M%S%3%6%9");
  EXPECT_EQ(res, YOGI_ERR_BUFFER_TOO_SMALL);
  std::string s4 = str;
  EXPECT_EQ(s4, "20090");
}

TEST(TimeTest, ParseTime) {
  long long timestamp;
  int res = YOGI_ParseTime(&timestamp, "2009-02-11T12:53:09.123Z", nullptr);
  EXPECT_EQ(res, YOGI_OK);
  EXPECT_EQ(timestamp, 1234356789123000000LL);

  res = YOGI_ParseTime(&timestamp, "20090211125309123456789", "%Y%m%d%H%M%S%3%6%9");
  EXPECT_EQ(res, YOGI_OK);
  EXPECT_EQ(timestamp, 1234356789123456789LL);

  res = YOGI_ParseTime(&timestamp, "01", "%S");
  EXPECT_EQ(res, YOGI_OK);
  EXPECT_EQ(timestamp, 2678401000000000LL);

  res = YOGI_ParseTime(&timestamp, "555", "%6");
  EXPECT_EQ(res, YOGI_OK);
  EXPECT_EQ(timestamp, 2678400000555000LL);

  res = YOGI_ParseTime(&timestamp, "123", "%X");
  EXPECT_EQ(res, YOGI_ERR_INVALID_TIME_FORMAT);

  res = YOGI_ParseTime(&timestamp, "123", nullptr);
  EXPECT_EQ(res, YOGI_ERR_PARSING_TIME_FAILED);
}
