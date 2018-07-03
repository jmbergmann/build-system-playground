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
    }
}
