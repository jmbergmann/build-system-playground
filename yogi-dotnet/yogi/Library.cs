using System;
using System.Runtime.InteropServices;

namespace Yogi
{
  public class Library
  {
    [DllImport("yogi-core.dll")]
    static extern IntPtr YOGI_GetVersion();

    public static string GetVersion() => Marshal.PtrToStringAnsi(YOGI_GetVersion());
  }
}
