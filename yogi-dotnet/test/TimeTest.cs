using System;
using Xunit;

namespace test
{
    public class TimeTest
    {
        [Fact]
        public void CurrentTime()
        {
            var t = Yogi.CurrentTime;
            Assert.IsType<DateTime>(t);
            var now = DateTime.UtcNow;
            Assert.InRange(now - t, TimeSpan.FromSeconds(-1), TimeSpan.FromSeconds(1));
        }
    }
}
