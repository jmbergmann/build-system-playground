using System;
using System.Runtime.InteropServices;

namespace Yogi
{
  public class Library
  {
    [DllImport("kernel32.dll")]
    private static extern IntPtr LoadLibrary(string dllToLoad);

    [DllImport("kernel32.dll")]
    private static extern IntPtr GetProcAddress(IntPtr hModule, string procedureName);

    [DllImport("kernel32.dll")]
    private static extern bool FreeLibrary(IntPtr hModule);

    [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
    private delegate IntPtr GetVersionDelegate();
    private static GetVersionDelegate YOGI_GetVersion;

    static Library()
    {
      IntPtr dll = LoadLibrary("yogi-core.dll");
      if (dll == IntPtr.Zero) {
        // TODO: error handling
      }

      IntPtr fn = GetProcAddress(dll, "YOGI_GetVersion");
      if (fn == IntPtr.Zero) {
        // TODO: error handling
      }

      YOGI_GetVersion = (GetVersionDelegate)Marshal.GetDelegateForFunctionPointer(fn, typeof(GetVersionDelegate));
    }

    public static string GetVersion() => Marshal.PtrToStringAnsi(YOGI_GetVersion());
  }
}
