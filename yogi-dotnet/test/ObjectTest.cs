using System;
using Xunit;

namespace test
{
    public class ObjectTest : TestCase
    {
        [Fact]
        public void Format()
        {
            var obj = new Yogi.Context();

            var s = obj.Format();
            Assert.Matches("Context \\[[1-9a-f][0-9a-f]+\\]", s);

            s = obj.Format("$T-[$X]");
            Assert.Matches("Context-\\[[1-9A-F][0-9A-F]+\\]", s);
        }

        [Fact]
        public void ToStringFormat()
        {
            var obj = new Yogi.Context();
            Assert.Matches("Context \\[[1-9a-f][0-9a-f]+\\]", obj.ToString());
        }
    }
}
