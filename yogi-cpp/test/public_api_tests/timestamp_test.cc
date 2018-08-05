#include "../common.h"

#include <chrono>
using namespace std::chrono_literals;

TEST(TimestampTest, Now) {
  EXPECT_GE(yogi::Timestamp::Now().DurationSinceEpoch(), 1e6s);
}

TEST(TimestampTest, Constructors) {
  EXPECT_EQ(yogi::Timestamp().DurationSinceEpoch(), 0ns);
  EXPECT_EQ(yogi::Timestamp(123ns).DurationSinceEpoch(), 123ns);
}

TEST(TimestampTest, DurationSinceEpoch) {
  EXPECT_EQ(yogi::Timestamp(123ns).DurationSinceEpoch(), 123ns);
}

TEST(TimestampTest, Nanosecond) {
  EXPECT_EQ(yogi::Timestamp(123456789ns).Nanosecond(), 789);
}

TEST(TimestampTest, Microsecond) {
  EXPECT_EQ(yogi::Timestamp(123456789ns).Microsecond(), 456);
}

TEST(TimestampTest, Millisecond) {
  EXPECT_EQ(yogi::Timestamp(123456789ns).Millisecond(), 123);
}

TEST(TimestampTest, Parse) {
  auto t = yogi::Timestamp::Parse("2009-02-11T12:53:09.123Z");
  EXPECT_EQ(t.DurationSinceEpoch(), 1234356789123000000ns);

  t = yogi::Timestamp::Parse("20090211125309123456789", "%Y%m%d%H%M%S%3%6%9");
  EXPECT_EQ(t.DurationSinceEpoch(), 1234356789123456789ns);
}

TEST(TimestampTest, Format) {
  auto t = yogi::Timestamp(std::chrono::nanoseconds(1234356789123456789));

  auto s = t.Format();
  EXPECT_EQ(s, "2009-02-11T12:53:09.123Z");

  s = t.Format("%Y%m%d%H%M%S%3%6%9");
  EXPECT_EQ(s, "20090211125309123456789");
}

TEST(TimestampTest, ToString) {
  auto t = yogi::Timestamp(std::chrono::nanoseconds(1234356789123456789ULL));
  EXPECT_EQ(t.ToString(), "2009-02-11T12:53:09.123Z");
}

TEST(TimestampTest, Operators) {
  auto t = yogi::Timestamp();
  t += 8ns;
  EXPECT_EQ(t.DurationSinceEpoch(), 8ns);
  t -= 1ns;
  EXPECT_EQ(t.DurationSinceEpoch(), 7ns);

  EXPECT_EQ(t + 5ns, yogi::Timestamp(12ns));
  EXPECT_EQ(t - 2ns, yogi::Timestamp(5ns));
  EXPECT_EQ(t - yogi::Timestamp(3ns), 4ns);

  EXPECT_TRUE(t == yogi::Timestamp(7ns));
  EXPECT_FALSE(t == yogi::Timestamp(8ns));

  EXPECT_FALSE(t != yogi::Timestamp(7ns));
  EXPECT_TRUE(t != yogi::Timestamp(8ns));

  EXPECT_TRUE(yogi::Timestamp(2ns) < yogi::Timestamp(3ns));
  EXPECT_FALSE(yogi::Timestamp(3ns) < yogi::Timestamp(2ns));

  EXPECT_FALSE(yogi::Timestamp(2ns) > yogi::Timestamp(3ns));
  EXPECT_TRUE(yogi::Timestamp(3ns) > yogi::Timestamp(2ns));

  EXPECT_TRUE(yogi::Timestamp(2ns) <= yogi::Timestamp(3ns));
  EXPECT_TRUE(yogi::Timestamp(3ns) <= yogi::Timestamp(3ns));
  EXPECT_FALSE(yogi::Timestamp(3ns) <= yogi::Timestamp(2ns));

  EXPECT_FALSE(yogi::Timestamp(2ns) >= yogi::Timestamp(3ns));
  EXPECT_TRUE(yogi::Timestamp(3ns) >= yogi::Timestamp(3ns));
  EXPECT_TRUE(yogi::Timestamp(3ns) >= yogi::Timestamp(2ns));
}
