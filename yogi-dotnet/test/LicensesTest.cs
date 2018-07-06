using System;
using Xunit;

namespace test
{
    public class LicensesTest : TestCase
    {
        [Fact]
        public void License()
        {
            var lic = Yogi.License;
            Assert.IsType<string>(lic);
            Assert.True(lic.Length > 100);
        }

        [Fact]
        public void ThirdPartyLicenses()
        {
            var lic = Yogi.ThirdPartyLicenses;
            Assert.IsType<string>(lic);
            Assert.True(lic.Length > 100);
            Assert.NotEqual(Yogi.License, lic);
        }
    }
}
