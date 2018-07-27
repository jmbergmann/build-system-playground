#include "../common.h"

#include <chrono>
using namespace std::chrono_literals;

TEST(TimeTest, Now) {
  EXPECT_GE(yogi::Timestamp::Now().NanosecondsSinceEpoch().count(), 1e15);
}

TEST(TimeTest, Constructors) {
  EXPECT_EQ(yogi::Timestamp().NanosecondsSinceEpoch(), 0ns);
  EXPECT_EQ(yogi::Timestamp(123ns).NanosecondsSinceEpoch(), 123ns);
}

TEST(TimeTest, NanosecondsSinceEpoch) {
  EXPECT_EQ(yogi::Timestamp(123ns).NanosecondsSinceEpoch(), 123ns);
}

TEST(TimeTest, NanosecondsFraction) {
  EXPECT_EQ(yogi::Timestamp(123456789ns).NanosecondsFraction(), 789);
}

TEST(TimeTest, MicrosecondsFraction) {
  EXPECT_EQ(yogi::Timestamp(123456789ns).MicrosecondsFraction(), 456);
}

TEST(TimeTest, MillisecondsFraction) {
  EXPECT_EQ(yogi::Timestamp(123456789ns).MillisecondsFraction(), 123);
}

TEST(TimeTest, ToString) {
  auto t = yogi::Timestamp(std::chrono::nanoseconds(1234356789123456789ULL));
  EXPECT_EQ(t.ToString(), "2009-02-11T12:53:09.123Z");
}

TEST(TimeTest, Operators) {
  auto t = yogi::Timestamp();
  t += 2ns;
  EXPECT_EQ(t.NanosecondsSinceEpoch(), 2ns);
  t -= 1ns;
  EXPECT_EQ(t.NanosecondsSinceEpoch(), 1ns);

  EXPECT_TRUE(t == yogi::Timestamp(1ns));
  EXPECT_FALSE(t == yogi::Timestamp(2ns));

  EXPECT_FALSE(t != yogi::Timestamp(1ns));
  EXPECT_TRUE(t != yogi::Timestamp(2ns));

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
