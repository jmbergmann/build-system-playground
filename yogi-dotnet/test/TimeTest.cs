using System;
using Xunit;

namespace test
{
    public class TimeTest : TestCase
    {
        [Fact]
        public void CurrentTime()
        {
            var t = Yogi.CurrentTime;
            Assert.IsType<DateTime>(t);
            Assert.Equal(DateTimeKind.Utc, t.Kind);
            var now = DateTime.UtcNow;
            Assert.InRange(now - t, TimeSpan.FromSeconds(-1), TimeSpan.FromSeconds(1));
        }

        [Fact]
        public void FormatTime()
        {
            var t = new DateTime(12343567891234567, DateTimeKind.Utc).AddYears(1969);

            var s = Yogi.FormatTime(t);
            Assert.IsType<string>(s);
            Assert.Equal("2009-02-11T12:53:09.123Z", s);

            s = Yogi.FormatTime(t, "%Y%m%d%H%M%S%3%6%9");
            Assert.Equal("20090211125309123456700", s);
        }

        [Fact]
        public void ParseTime()
        {
            var t = Yogi.ParseTime("2009-02-11T12:53:09.123Z");

            Assert.IsType<DateTime>(t);
            Assert.Equal(DateTimeKind.Utc, t.Kind);
            Assert.Equal(12343567891230000, t.AddYears(-1969).Ticks);

            t = Yogi.ParseTime("20090211125309123456789", "%Y%m%d%H%M%S%3%6%9");
            Assert.Equal(12343567891234567, t.AddYears(-1969).Ticks);
        }
    }
}
