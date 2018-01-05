using NUnit.Framework;
using System;
using System.Reflection;
using Yogi;

namespace test
{
  [TestFixture]
  public class LibraryTest
  {
    [Test]
    public void Version() => Assert.IsNotEmpty(Yogi.Library.GetVersion());

    [Test]
    public void AssemblyVersion()
    {
      var v = Assembly.GetAssembly(typeof(Yogi.Library)).GetName().Version;
      var s = string.Format("{0}.{1}.{2}", v.Major, v.Minor, v.Build);
      Assert.AreEqual(Yogi.Library.GetVersion(), s);
    }
  }
}
