using System;
using System.Reflection;
using Xunit;

namespace test
{
    public class VersionTest : TestCase
    {
        [Fact]
        public void Version()
        {
            Assert.True(Yogi.Version.Length > 4);
        }

        [Fact]
        public void AssemblyVersion()
        {
            var v = Assembly.GetAssembly(typeof(Yogi)).GetName().Version;
            var s = string.Format("{0}.{1}.{2}", v.Major, v.Minor, v.Build);
            Assert.Equal(Yogi.Version, s);
        }
    }
}
