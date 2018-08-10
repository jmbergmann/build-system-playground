#include "../common.h"

using yogi::ArithmeticException;
using yogi::Duration;
using yogi::Timestamp;
using namespace std::chrono_literals;

TEST(TimestampTest, FromDurationSinceEpoch) {
  EXPECT_EQ(Timestamp::FromDurationSinceEpoch(123ns).DurationSinceEpoch(),
            123ns);
  EXPECT_THROW(Timestamp::FromDurationSinceEpoch(Duration::kInfinity),
               ArithmeticException);
  EXPECT_THROW(Timestamp::FromDurationSinceEpoch(yogi::Duration(-1ns)),
               ArithmeticException);
}

TEST(TimestampTest, Now) {
  EXPECT_GE(Timestamp::Now().DurationSinceEpoch(), 1e6s);
}

TEST(TimestampTest, Parse) {
  auto t = Timestamp::Parse("2009-02-11T12:53:09.123Z");
  EXPECT_EQ(t.DurationSinceEpoch(), 1234356789123000000ns);

  t = Timestamp::Parse("20090211125309123456789", "%Y%m%d%H%M%S%3%6%9");
  EXPECT_EQ(t.DurationSinceEpoch(), 1234356789123456789ns);
}

TEST(TimestampTest, Constructor) {
  EXPECT_EQ(Timestamp().DurationSinceEpoch(), 0ns);
}

TEST(TimestampTest, DurationSinceEpoch) {
  EXPECT_EQ(Timestamp::FromDurationSinceEpoch(123ns).DurationSinceEpoch(),
            123ns);
}

TEST(TimestampTest, Nanoseconds) {
  EXPECT_EQ(Timestamp::FromDurationSinceEpoch(123456789ns).Nanoseconds(), 789);
}

TEST(TimestampTest, Microseconds) {
  EXPECT_EQ(Timestamp::FromDurationSinceEpoch(123456789ns).Microseconds(), 456);
}

TEST(TimestampTest, Milliseconds) {
  EXPECT_EQ(Timestamp::FromDurationSinceEpoch(123456789ns).Milliseconds(), 123);
}

TEST(TimestampTest, Format) {
  auto t = Timestamp::FromDurationSinceEpoch(1234356789123456789ns);

  auto s = t.Format();
  EXPECT_EQ(s, "2009-02-11T12:53:09.123Z");

  s = t.Format("%Y%m%d%H%M%S%3%6%9");
  EXPECT_EQ(s, "20090211125309123456789");
}

TEST(TimestampTest, ToString) {
  auto t = Timestamp::FromDurationSinceEpoch(1234356789123456789ns);
  EXPECT_EQ(t.ToString(), "2009-02-11T12:53:09.123Z");
}

TEST(TimestampTest, Operators) {
  auto t = Timestamp();
  t += 8ns;
  EXPECT_EQ(t.DurationSinceEpoch(), 8ns);
  EXPECT_THROW(t += -1ms, ArithmeticException);
  t -= 1ns;
  EXPECT_EQ(t.DurationSinceEpoch(), 7ns);
  EXPECT_THROW(t -= 1ms, ArithmeticException);

  EXPECT_EQ(t + 5ns, Timestamp::FromDurationSinceEpoch(12ns));
  EXPECT_THROW(t + -1ms, ArithmeticException);
  EXPECT_EQ(t - 2ns, Timestamp::FromDurationSinceEpoch(5ns));
  EXPECT_EQ(t - Timestamp::FromDurationSinceEpoch(3ns), 4ns);
  EXPECT_THROW(t - 1ms, ArithmeticException);

  EXPECT_TRUE(t == Timestamp::FromDurationSinceEpoch(7ns));
  EXPECT_FALSE(t == Timestamp::FromDurationSinceEpoch(8ns));

  EXPECT_FALSE(t != Timestamp::FromDurationSinceEpoch(7ns));
  EXPECT_TRUE(t != Timestamp::FromDurationSinceEpoch(8ns));

  EXPECT_TRUE(Timestamp::FromDurationSinceEpoch(2ns) <
              Timestamp::FromDurationSinceEpoch(3ns));
  EXPECT_FALSE(Timestamp::FromDurationSinceEpoch(3ns) <
               Timestamp::FromDurationSinceEpoch(2ns));
  EXPECT_FALSE(Timestamp::FromDurationSinceEpoch(3ns) <
               Timestamp::FromDurationSinceEpoch(3ns));

  EXPECT_FALSE(Timestamp::FromDurationSinceEpoch(2ns) >
               Timestamp::FromDurationSinceEpoch(3ns));
  EXPECT_TRUE(Timestamp::FromDurationSinceEpoch(3ns) >
              Timestamp::FromDurationSinceEpoch(2ns));
  EXPECT_FALSE(Timestamp::FromDurationSinceEpoch(3ns) >
               Timestamp::FromDurationSinceEpoch(3ns));

  EXPECT_TRUE(Timestamp::FromDurationSinceEpoch(2ns) <=
              Timestamp::FromDurationSinceEpoch(3ns));
  EXPECT_TRUE(Timestamp::FromDurationSinceEpoch(3ns) <=
              Timestamp::FromDurationSinceEpoch(3ns));
  EXPECT_FALSE(Timestamp::FromDurationSinceEpoch(3ns) <=
               Timestamp::FromDurationSinceEpoch(2ns));

  EXPECT_FALSE(Timestamp::FromDurationSinceEpoch(2ns) >=
               Timestamp::FromDurationSinceEpoch(3ns));
  EXPECT_TRUE(Timestamp::FromDurationSinceEpoch(3ns) >=
              Timestamp::FromDurationSinceEpoch(3ns));
  EXPECT_TRUE(Timestamp::FromDurationSinceEpoch(3ns) >=
              Timestamp::FromDurationSinceEpoch(2ns));
}
