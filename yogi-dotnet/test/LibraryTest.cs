using System;
using System.Reflection;
using Xunit;
using Yogi;

namespace test
{
  public class LibraryTest
  {
    [Fact]
    public void Version()
    {
      Assert.True(Yogi.Library.GetVersion().Length > 3);
    }

    [Fact]
    public void AssemblyVersion()
    {
      var v = Assembly.GetAssembly(typeof(Yogi.Library)).GetName().Version;
      var s = string.Format("{0}.{1}.{2}", v.Major, v.Minor, v.Build);
      Assert.Equal(Yogi.Library.GetVersion(), s);
    }
  }
}
