#include "../common.h"

#include <chrono>

using yogi::internal::ExtractFromJson;
using namespace std::chrono_literals;
using namespace std::string_literals;

// TEST(JsonTest, ExtractInt) {
//   auto json = R"({"int":123})";
//   EXPECT_EQ(ExtractFromJson<int>(json, "int"), 123);

//   json = R"({"abc":"xyz", "int":123})";
//   EXPECT_EQ(ExtractFromJson<int>(json, "int"), 123);

//   json = R"({"int":123, "abc":555})";
//   EXPECT_EQ(ExtractFromJson<int>(json, "int"), 123);
// }

// TEST(JsonTest, ExtractFloat) {
//   auto json = R"({"flt":12.34})";
//   EXPECT_FLOAT_EQ(ExtractFromJson<float>(json, "flt"), 12.34);

//   json = R"({"abc":"xyz", "flt":12.34})";
//   EXPECT_FLOAT_EQ(ExtractFromJson<float>(json, "flt"), 12.34);

//   json = R"({"flt":12.34, "abc":555})";
//   EXPECT_FLOAT_EQ(ExtractFromJson<float>(json, "flt"), 12.34);
// }

// TEST(JsonTest, ExtractString) {
//   auto json = R"({"str":"hi you"})";
//   EXPECT_EQ(ExtractFromJson<std::string>(json, "str"), "hi you");

//   auto json = R"({"abc":"xyz", "str":"hi you"})";
//   EXPECT_EQ(ExtractFromJson<std::string>(json, "str"), "hi you");

//   auto json = R"({"str":"hi you", "abc":555})";
//   EXPECT_EQ(ExtractFromJson<std::string>(json, "str"), "hi you");

//   auto json = R"({"str":"\b\f\n\r\t\"\\})";
//   EXPECT_EQ(ExtractFromJson<std::string>(json, "str"), "\b\f\n\r\t\"\\");
// }

// TEST(JsonTest, ExtractDuration) {
//   auto json = R"({"dur":0.001})";
//   EXPECT_EQ(ExtractFromJson<std::chrono::nanoseconds>(json, "dur").count(),
//             1ms);
// }

// TEST(JsonTest, ExtractTimestamp) {
//   auto json = R"({"time":"2009-02-11T12:53:09.123Z")})";
//   EXPECT_EQ(
//       ExtractFromJson<yogi::Timestamp>(json, "time").NanosecondsSinceEpoch(),
//       1234356789123456789LL);
// }
