using System;
using System.Runtime.InteropServices;

static public partial class Yogi
{
    internal partial class Api
    {
        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate IntPtr GetVersionDelegate();
        public static GetVersionDelegate YOGI_GetVersion
            = Library.GetDelegateForFunction<GetVersionDelegate>("YOGI_GetVersion");
    }

    public static string GetVersion()
    {
        return Marshal.PtrToStringAnsi(Api.YOGI_GetVersion());
    }
}
