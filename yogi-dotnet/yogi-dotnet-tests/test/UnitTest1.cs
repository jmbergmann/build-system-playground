using System;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using Yogi;

namespace yogi_dotnet_tests
{
    [TestClass]
    public class UnitTest1
    {
        [TestMethod]
        public void TestMethod1()
        {
            Assert.AreEqual("0.0.3-alpha", Yogi.Version.GetVersion());
        }
    }
}
