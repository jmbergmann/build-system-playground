#include "../common.h"

#include <chrono>

using namespace yogi::internal;
using namespace std::chrono_literals;
using namespace std::string_literals;

TEST(JsonTest, ExtractInt) {
  auto json = R"({"int":123})";
  EXPECT_EQ(ExtractIntFromJson(json, "int"), 123);

  json = R"({"abc":"xyz", "int":123})";
  EXPECT_EQ(ExtractIntFromJson(json, "int"), 123);

  json = R"({"int":123, "abc":555})";
  EXPECT_EQ(ExtractIntFromJson(json, "int"), 123);
}

TEST(JsonTest, ExtractFloat) {
  auto json = R"({"flt":12.34})";
  EXPECT_FLOAT_EQ(ExtractFloatFromJson(json, "flt"), 12.34f);

  json = R"({"abc":"xyz", "flt":12.34})";
  EXPECT_FLOAT_EQ(ExtractFloatFromJson(json, "flt"), 12.34f);

  json = R"({"flt":12.34, "abc":555})";
  EXPECT_FLOAT_EQ(ExtractFloatFromJson(json, "flt"), 12.34f);
}

TEST(JsonTest, ExtractString) {
  auto json = R"({"str":"hi you"})";
  EXPECT_EQ(ExtractStringFromJson(json, "str"), "hi you");

  json = R"({"abc":"xyz", "str":"hi you"})";
  EXPECT_EQ(ExtractStringFromJson(json, "str"), "hi you");

  json = R"({"str":"hi\"you", "abc":555})";
  EXPECT_EQ(ExtractStringFromJson(json, "str"), "hi\"you");

  json = R"({"str":"\b\f\n\r\t\"\\"})";
  EXPECT_EQ(ExtractStringFromJson(json, "str"), "\b\f\n\r\t\"\\");
}

TEST(JsonTest, ExtractDuration) {
  auto json = R"({"dur":0.00123})";
  EXPECT_EQ(ExtractDurationFromJson(json, "dur"), 1230us);

  json = R"({"dur":-1})";
  EXPECT_EQ(ExtractDurationFromJson(json, "dur"), yogi::Duration::kInfinity);
}

TEST(JsonTest, ExtractTimestamp) {
  auto json = R"({"time":"2009-02-11T12:53:09.123Z")})";
  EXPECT_EQ(ExtractTimestampFromJson(json, "time")
                .DurationSinceEpoch()
                .NanosecondsCount(),
            1234356789123000000LL);
}
