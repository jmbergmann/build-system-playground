using System;
using Xunit;

namespace test
{
    public class ObjectTest : TestCase
    {
        [Fact]
        public void ToStringFormat()
        {
            var obj = new Yogi.Context();
            Assert.Matches("Context \\[[1-9a-f][0-9a-f]+\\]", obj.ToString());
        }
    }
}
