#include "../common.h"

#include <chrono>

using namespace yogi::internal;
using namespace std::chrono_literals;
using namespace std::string_literals;

TEST(ConversionTest, DurationToCoreDuration) {
  EXPECT_EQ(DurationToCoreDuration(123us), 123000);
  EXPECT_EQ(DurationToCoreDuration((std::chrono::nanoseconds::max)()), -1);
}

TEST(ConversionTest, CoreDurationToDuration) {
  EXPECT_EQ(CoreDurationToDuration(123000), 123us);
  EXPECT_EQ(CoreDurationToDuration(-1), (std::chrono::nanoseconds::max)());
}

TEST(ConversionTest, StringToCoreString) {
  const char* str = "";
  EXPECT_EQ(StringToCoreString(str), str);

  std::string s = "abc";
  EXPECT_EQ(StringToCoreString(s), s.c_str());
}
