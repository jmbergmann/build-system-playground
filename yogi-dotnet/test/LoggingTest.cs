using System;
using System.Reflection;
using Xunit;

namespace test
{
    public class LoggingTest
    {
        [Fact]
        public void VerbosityEnum()
        {
            Helpers.AssertEnumMatches<Yogi.Verbosity>("YOGI_VB_");
        }

        [Fact]
        public void StreamEnum()
        {
            Helpers.AssertEnumMatches<Yogi.Stream>("YOGI_ST_");
        }
    }
}
