using System;
using System.Runtime.InteropServices;
using System.Runtime.CompilerServices;

static public partial class Yogi
{
    internal partial class LibraryFunctions
    {
        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate IntPtr GetVersionDelegate();
        public static GetVersionDelegate YOGI_GetVersion;

        static LibraryFunctions()
        {
            RuntimeHelpers.RunClassConstructor(typeof(Library).TypeHandle);

            IntPtr fn = Library.utils.GetProcAddress(Library.dll, "YOGI_GetVersion");
            if (fn == IntPtr.Zero)
            {
                throw new MissingMethodException($"Function YOGI_GetVersion is missing in {Library.filename}");
            }

            YOGI_GetVersion = (GetVersionDelegate)Marshal.GetDelegateForFunctionPointer(fn, typeof(GetVersionDelegate));
        }
    }

    public static string GetVersion()
    {
        return Marshal.PtrToStringAnsi(LibraryFunctions.YOGI_GetVersion());
    }
}
