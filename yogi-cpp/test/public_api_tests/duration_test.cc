#include "../common.h"

#include <chrono>
using namespace std::chrono_literals;

using yogi::ArithmeticException;
using yogi::Duration;

TEST(DurationTest, Zero) {
  auto dur = Duration::kZero;
  EXPECT_FALSE(dur.IsInfinite());
  EXPECT_EQ(dur.TotalNanoseconds(), 0);
}

TEST(DurationTest, Infinity) {
  auto dur = Duration::kInfinity;
  EXPECT_TRUE(dur.IsInfinite());
  EXPECT_TRUE(dur > Duration::kZero);
}

TEST(DurationTest, NegativeInfinity) {
  auto dur = Duration::kNegativeInfinity;
  EXPECT_TRUE(dur.IsInfinite());
  EXPECT_TRUE(dur < Duration::kZero);
}

TEST(DurationTest, FromNanoseconds) {
  auto dur = Duration::FromNanoseconds(123);
  EXPECT_FALSE(dur.IsInfinite());
  EXPECT_EQ(dur.TotalNanoseconds(), 123);
}

TEST(DurationTest, FromMicroseconds) {
  auto dur = Duration::FromMicroseconds(123);
  EXPECT_FALSE(dur.IsInfinite());
  EXPECT_EQ(dur.TotalMicroseconds(), 123);
}

TEST(DurationTest, FromMilliseconds) {
  auto dur = Duration::FromMilliseconds(123);
  EXPECT_FALSE(dur.IsInfinite());
  EXPECT_EQ(dur.TotalMilliseconds(), 123);
}

TEST(DurationTest, FromSeconds) {
  auto dur = Duration::FromSeconds(123);
  EXPECT_FALSE(dur.IsInfinite());
  EXPECT_EQ(dur.TotalSeconds(), 123);
}

TEST(DurationTest, FromMinutes) {
  auto dur = Duration::FromMinutes(123);
  EXPECT_FALSE(dur.IsInfinite());
  EXPECT_EQ(dur.TotalMinutes(), 123);
}

TEST(DurationTest, FromHours) {
  auto dur = Duration::FromHours(123);
  EXPECT_FALSE(dur.IsInfinite());
  EXPECT_EQ(dur.TotalHours(), 123);
}

TEST(DurationTest, FromDays) {
  auto dur = Duration::FromDays(123);
  EXPECT_FALSE(dur.IsInfinite());
  EXPECT_EQ(dur.TotalDays(), 123);
}

TEST(DurationTest, DefaultConstructor) {
  auto dur = Duration();
  EXPECT_FALSE(dur.IsInfinite());
  EXPECT_EQ(dur.TotalNanoseconds(), 0);
}

TEST(DurationTest, ChronoConstructor) {
  auto dur = Duration(12s);
  EXPECT_FALSE(dur.IsInfinite());
  EXPECT_EQ(dur.TotalSeconds(), 12);

  dur = Duration(33ms);
  EXPECT_FALSE(dur.IsInfinite());
  EXPECT_EQ(dur.TotalMilliseconds(), 33);
}

TEST(DurationTest, Nanoseconds) {
  auto dur = Duration(123456789132465887ns);
  EXPECT_EQ(dur.Nanoseconds(), 887);
}

TEST(DurationTest, Microseconds) {
  auto dur = Duration(123456789132465887ns);
  EXPECT_EQ(dur.Microseconds(), 465);
}

TEST(DurationTest, Milliseconds) {
  auto dur = Duration(123456789132465887ns);
  EXPECT_EQ(dur.Milliseconds(), 132);
}

TEST(DurationTest, Seconds) {
  auto dur = Duration(123456789132465887ns);
  EXPECT_EQ(dur.Seconds(), 9);
}

TEST(DurationTest, Minutes) {
  auto dur = Duration(123456789132465887ns);
  EXPECT_EQ(dur.Minutes(), 33);
}

TEST(DurationTest, Hours) {
  auto dur = Duration(123456789132465887ns);
  EXPECT_EQ(dur.Hours(), 21);
}

TEST(DurationTest, Days) {
  auto dur = Duration(123456789132465887ns);
  EXPECT_EQ(dur.Days(), 1428);
}

TEST(DurationTest, TotalNanoseconds) {
  auto dur = Duration(123456789132465887ns);
  EXPECT_EQ(dur.TotalNanoseconds(), 123456789132465887ll);
}

TEST(DurationTest, TotalMicroseconds) {
  auto dur = Duration(123456789132465887ns);
  EXPECT_EQ(dur.TotalMicroseconds(), 123456789132465ll);
}

TEST(DurationTest, TotalMilliseconds) {
  auto dur = Duration(123456789132465887ns);
  EXPECT_EQ(dur.TotalMilliseconds(), 123456789132ll);
}

TEST(DurationTest, TotalSeconds) {
  auto dur = Duration(123456789132465887ns);
  EXPECT_EQ(dur.TotalSeconds(), 123456789ll);
}

TEST(DurationTest, TotalMinutes) {
  auto dur = Duration(123456789132465887ns);
  EXPECT_EQ(dur.TotalMinutes(), 2057613ll);
}

TEST(DurationTest, TotalHours) {
  auto dur = Duration(123456789132465887ns);
  EXPECT_EQ(dur.TotalHours(), 34293ll);
}

TEST(DurationTest, TotalDays) {
  auto dur = Duration(123456789132465887ns);
  EXPECT_EQ(dur.TotalDays(), 1428ll);
}

TEST(DurationTest, Negated) {
  auto dur = Duration(123ns);
  auto neg_dur = dur.Negated();
  EXPECT_EQ(neg_dur.TotalNanoseconds(), -dur.TotalNanoseconds());
  EXPECT_FALSE(neg_dur.IsInfinite());
}

TEST(DurationTest, ToChronoDuration) {
  auto dur = Duration(123456789132465887ns);
  EXPECT_EQ(dur.ToChronoDuration<>(), 123456789132465887ns);
  EXPECT_EQ(dur.ToChronoDuration<std::chrono::seconds>(), 123456789s);
  EXPECT_EQ(Duration::kInfinity.ToChronoDuration<>(),
            (std::chrono::nanoseconds::max)());
  EXPECT_EQ(Duration::kInfinity.ToChronoDuration<std::chrono::minutes>(),
            (std::chrono::minutes::max)());
  EXPECT_EQ(Duration::kNegativeInfinity.ToChronoDuration<>(),
            (std::chrono::nanoseconds::min)());
  EXPECT_EQ(
      Duration::kNegativeInfinity.ToChronoDuration<std::chrono::minutes>(),
      (std::chrono::minutes::min)());
}

TEST(DurationTest, IsInfinite) {
  EXPECT_FALSE(Duration::kZero.IsInfinite());
  EXPECT_FALSE(Duration(123ns).IsInfinite());
  EXPECT_TRUE(Duration::kInfinity.IsInfinite());
  EXPECT_TRUE(Duration::kNegativeInfinity.IsInfinite());
}

TEST(DurationTest, Format) {
  auto dur = Duration(123456789123456789ns);

  auto s = dur.Format();
  EXPECT_EQ(s, "1428d 21:33:09.123456789");

  s = dur.Format("%S");
  EXPECT_EQ(s, "09");

  s = Duration::kInfinity.Format("%S", "abc");
  EXPECT_EQ(s, "abc");

  s = Duration::kInfinity.Format();
  EXPECT_EQ(s, "inf");

  s = Duration::kNegativeInfinity.Format();
  EXPECT_EQ(s, "-inf");

  s = Duration::kInfinity.Format("%9", "%+bla");
  EXPECT_EQ(s, "+bla");
}

TEST(DurationTest, ToString) {
  auto dur = Duration(123456789123456789ns);

  auto s = dur.ToString();
  EXPECT_EQ(s, "1428d 21:33:09.123456789");
}

TEST(DurationTest, PlusOperator) {
  auto dur = Duration(10ns);
  auto dur2 = Duration(3us);
  EXPECT_EQ((dur + dur2).TotalNanoseconds(), 3010);

  EXPECT_EQ(dur + Duration::kInfinity, Duration::kInfinity);
  EXPECT_EQ(Duration::kInfinity + Duration::kInfinity, Duration::kInfinity);
  EXPECT_EQ(dur + Duration::kNegativeInfinity, Duration::kNegativeInfinity);
  EXPECT_EQ(Duration::kNegativeInfinity + Duration::kNegativeInfinity,
            Duration::kNegativeInfinity);
  EXPECT_THROW(Duration::kInfinity + Duration::kNegativeInfinity,
               ArithmeticException);
}

TEST(DurationTest, MinusOperator) {
  auto dur1 = Duration(10ns);
  auto dur2 = Duration(3us);
  EXPECT_EQ((dur1 - dur2).TotalNanoseconds(), -2990);

  EXPECT_EQ(dur1 - Duration::kInfinity, Duration::kNegativeInfinity);
  EXPECT_EQ(Duration::kInfinity - Duration::kNegativeInfinity,
            Duration::kInfinity);
  EXPECT_EQ(dur1 - Duration::kNegativeInfinity, Duration::kInfinity);
  EXPECT_EQ(Duration::kNegativeInfinity - Duration::kInfinity,
            Duration::kNegativeInfinity);
  EXPECT_THROW(Duration::kNegativeInfinity - Duration::kNegativeInfinity,
               ArithmeticException);
}

TEST(DurationTest, MultiplicationOperator) {
  auto dur = Duration(8ns);

  EXPECT_EQ((dur * 3).TotalNanoseconds(), 24);
  EXPECT_EQ((dur * 3.5).TotalNanoseconds(), 28);

  EXPECT_EQ(Duration::kInfinity * 5, Duration::kInfinity);
  EXPECT_EQ(Duration::kInfinity * -5, Duration::kNegativeInfinity);
  EXPECT_THROW(Duration::kInfinity * 0, ArithmeticException);
}

TEST(DurationTest, DivisionOperator) {
  auto dur = Duration(28ns);

  EXPECT_EQ((dur / 2).TotalNanoseconds(), 14);
  EXPECT_EQ((dur / 3.5).TotalNanoseconds(), 8);

  EXPECT_EQ(Duration::kInfinity / 5, Duration::kInfinity);
  EXPECT_EQ(Duration::kInfinity / -5, Duration::kNegativeInfinity);
  EXPECT_THROW(Duration::kInfinity / 0, ArithmeticException);
}

TEST(DurationTest, PlusEqualsOperator) {
  auto dur1 = Duration(10ns);
  auto dur2 = Duration(3us);
  auto dur3 = dur1 += dur2;
  EXPECT_EQ(dur1, dur3);
  EXPECT_EQ(dur1.TotalNanoseconds(), 3010);
}

TEST(DurationTest, MinusEqualsOperator) {
  auto dur1 = Duration(10ns);
  auto dur2 = Duration(3us);
  auto dur3 = dur1 -= dur2;
  EXPECT_EQ(dur1, dur3);
  EXPECT_EQ(dur1.TotalNanoseconds(), -2990);
}

TEST(DurationTest, MultiplyEqualsOperator) {
  auto dur1 = Duration(3ns);
  auto dur2 = dur1 *= 3;
  EXPECT_EQ(dur1, dur2);
  EXPECT_EQ(dur1.TotalNanoseconds(), 9);
}

TEST(DurationTest, DivideEqualsOperator) {
  auto dur1 = Duration(12ns);
  auto dur2 = dur1 /= 3;
  EXPECT_EQ(dur1, dur2);
  EXPECT_EQ(dur1.TotalNanoseconds(), 4);
}

TEST(DurationTest, EqualityOperator) {
  auto dur1 = Duration(1000ns);
  auto dur2 = Duration(1us);
  auto dur3 = Duration(2ns);
  EXPECT_TRUE(dur1 == dur2);
  EXPECT_FALSE(dur1 == dur3);
}

TEST(DurationTest, NotEqualsOperator) {
  auto dur1 = Duration(1000ns);
  auto dur2 = Duration(1us);
  auto dur3 = Duration(2ns);
  EXPECT_FALSE(dur1 != dur2);
  EXPECT_TRUE(dur1 != dur3);
}

TEST(DurationTest, LessThanOperator) {
  auto dur1 = Duration(1ns);
  auto dur2 = Duration(1ns);
  auto dur3 = Duration(2ns);
  EXPECT_FALSE(dur1 < dur2);
  EXPECT_TRUE(dur2 < dur3);
  EXPECT_FALSE(dur3 < dur1);
}

TEST(DurationTest, GreaterThanOperator) {
  auto dur1 = Duration(1ns);
  auto dur2 = Duration(1ns);
  auto dur3 = Duration(2ns);
  EXPECT_FALSE(dur1 > dur2);
  EXPECT_FALSE(dur2 > dur3);
  EXPECT_TRUE(dur3 > dur1);
}

TEST(DurationTest, LessOrEqualOperator) {
  auto dur1 = Duration(1ns);
  auto dur2 = Duration(1ns);
  auto dur3 = Duration(2ns);
  EXPECT_TRUE(dur1 <= dur2);
  EXPECT_TRUE(dur2 <= dur3);
  EXPECT_FALSE(dur3 <= dur1);
}

TEST(DurationTest, GreaterOrEqualOperator) {
  auto dur1 = Duration(1ns);
  auto dur2 = Duration(1ns);
  auto dur3 = Duration(2ns);
  EXPECT_TRUE(dur1 >= dur2);
  EXPECT_FALSE(dur2 >= dur3);
  EXPECT_TRUE(dur3 >= dur1);
}
