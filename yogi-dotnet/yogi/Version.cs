using System;
using System.Runtime.InteropServices;

public static partial class Yogi
{
    partial class Api
    {
        // === YOGI_GetVersion ===
        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate IntPtr GetVersionDelegate();

        public static GetVersionDelegate YOGI_GetVersion
            = Library.GetDelegateForFunction<GetVersionDelegate>(
                "YOGI_GetVersion");
    }

    /// <summary>
    /// Version string of the loaded Yogi Core library.
    /// </summary>
    public static string Version
    {
        get
        {
            return Marshal.PtrToStringAnsi(Api.YOGI_GetVersion());
        }
    }
}
