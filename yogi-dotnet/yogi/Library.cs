using System;
using System.Runtime.InteropServices;

namespace Yogi
{
  public class Library
  {
    private interface LibUtils
    {
      IntPtr LoadLibrary(string filename);
      void FreeLibrary(IntPtr libHandle);
      IntPtr GetProcAddress(IntPtr libHandle, string fnName);
    }

    private class WindowsLibUtils : LibUtils
    {
      void LibUtils.FreeLibrary(IntPtr handle)
      {
        FreeLibrary(handle);
      }

      IntPtr LibUtils.GetProcAddress(IntPtr dllHandle, string name)
      {
        return GetProcAddress(dllHandle, name);
      }

      IntPtr LibUtils.LoadLibrary(string fileName)
      {
        return LoadLibrary(fileName);
      }

      [DllImport("kernel32")]
      private static extern IntPtr LoadLibrary(string fileName);

      [DllImport("kernel32.dll")]
      private static extern int FreeLibrary(IntPtr handle);

      [DllImport("kernel32.dll")]
      private static extern IntPtr GetProcAddress(IntPtr handle, string procedureName);
    }

    private class UnixLibUtils : LibUtils
    {
      public IntPtr LoadLibrary(string fileName)
      {
        return dlopen(fileName, RTLD_NOW);
      }

      public void FreeLibrary(IntPtr handle)
      {
        dlclose(handle);
      }

      public IntPtr GetProcAddress(IntPtr dllHandle, string name)
      {
        // clear previous errors if any
        dlerror();
        var res = dlsym(dllHandle, name);
        var errPtr = dlerror();
        if (errPtr != IntPtr.Zero)
        {
          throw new Exception("dlsym: " + Marshal.PtrToStringAnsi(errPtr));
        }
        return res;
      }

      const int RTLD_NOW = 2;

      [DllImport("libdl.so")]
      private static extern IntPtr dlopen(String fileName, int flags);

      [DllImport("libdl.so")]
      private static extern IntPtr dlsym(IntPtr handle, String symbol);

      [DllImport("libdl.so")]
      private static extern int dlclose(IntPtr handle);

      [DllImport("libdl.so")]
      private static extern IntPtr dlerror();
    }

    [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
    private delegate IntPtr GetVersionDelegate();
    private static GetVersionDelegate YOGI_GetVersion;

    static Library()
    {
      LibUtils utils = RuntimeInformation.IsOSPlatform(OSPlatform.Windows)
                     ? (LibUtils) new WindowsLibUtils()
                     : (LibUtils) new UnixLibUtils();

      string filename = Environment.GetEnvironmentVariable("YOGI_CORE_LIBRARY");
      if (filename == null) {
        filename = "yogi-core.dll";
      }

      IntPtr dll = utils.LoadLibrary(filename);
      if (dll == IntPtr.Zero) {
        throw new DllNotFoundException($"Could not load library {filename}");
      }

      IntPtr fn = utils.GetProcAddress(dll, "YOGI_GetVersion");
      if (fn == IntPtr.Zero) {
        throw new MissingMethodException($"Function YOGI_GetVersion is missing in {filename}");
      }

      YOGI_GetVersion = (GetVersionDelegate)Marshal.GetDelegateForFunctionPointer(fn, typeof(GetVersionDelegate));
    }

    public static string GetVersion() => Marshal.PtrToStringAnsi(YOGI_GetVersion());
  }
}
