using System;
using Xunit;

namespace test
{
    public class TimeTest : TestCase
    {
        [Fact]
        public void CurrentTime()
        {
            var start = Yogi.Timestamp.Now;
            var t = Yogi.CurrentTime;
            Assert.IsType<Yogi.Timestamp>(t);
            Assert.InRange(t, start, Yogi.Timestamp.Now);
        }
    }
}
