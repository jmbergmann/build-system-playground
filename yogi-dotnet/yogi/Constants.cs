using System;
using System.Runtime.InteropServices;

static public partial class Yogi
{
    internal partial class Api
    {
        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate int GetIntConstantDelegate(ref int dest, int constant);
        public static GetIntConstantDelegate YOGI_GetIntConstant
            = Library.GetDelegateForFunction<GetIntConstantDelegate>("YOGI_GetConstant");

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate int GetLongLongConstantDelegate(ref long dest, int constant);
        public static GetLongLongConstantDelegate YOGI_GetLongLongConstant
            = Library.GetDelegateForFunction<GetLongLongConstantDelegate>("YOGI_GetConstant");

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate int GetStringConstantDelegate(ref IntPtr dest, int constant);
        public static GetStringConstantDelegate YOGI_GetStringConstant
            = Library.GetDelegateForFunction<GetStringConstantDelegate>("YOGI_GetConstant");
    }

    static public class Constants
    {
        static public readonly string VersionNumber;
        static public readonly int VersionMajor;
        static public readonly int VersionMinor;
        static public readonly int VersionPatch;
        static public readonly string DefaultAdvAddress;
        static public readonly int DefaultAdvPort;
        static public readonly TimeSpan DefaultAdvInterval;
        static public readonly TimeSpan DefaultConnectionTimeout;
        static public readonly Verbosity DefaultLoggerVerbosity;
        static public readonly string DefaultLogTimeFormat;
        static public readonly string DefaultLogFormat;
        static public readonly int MaxMessageSize;

        static Constants()
        {
            // TODO: Handle return value
            IntPtr str = new IntPtr();
            Api.YOGI_GetStringConstant(ref str, 1);
            VersionNumber = Marshal.PtrToStringAnsi(str);

            Api.YOGI_GetIntConstant(ref VersionMajor, 2);
            Api.YOGI_GetIntConstant(ref VersionMinor, 3);
            Api.YOGI_GetIntConstant(ref VersionPatch, 4);

            Api.YOGI_GetStringConstant(ref str, 5);
            DefaultAdvAddress = Marshal.PtrToStringAnsi(str);

            Api.YOGI_GetIntConstant(ref DefaultAdvPort, 6);

            long t = -1;
            Api.YOGI_GetLongLongConstant(ref t, 7);
            DefaultAdvInterval = TimeSpan.FromMilliseconds(Convert.ToDouble(t) / 1e6);

            Api.YOGI_GetLongLongConstant(ref t, 8);
            DefaultConnectionTimeout = TimeSpan.FromMilliseconds(Convert.ToDouble(t) / 1e6);

            int n = -1;
            Api.YOGI_GetIntConstant(ref n, 9);
            DefaultLoggerVerbosity = (Verbosity)n;

            Api.YOGI_GetStringConstant(ref str, 10);
            DefaultLogTimeFormat = Marshal.PtrToStringAnsi(str);

            Api.YOGI_GetStringConstant(ref str, 11);
            DefaultLogFormat = Marshal.PtrToStringAnsi(str);

            Api.YOGI_GetIntConstant(ref MaxMessageSize, 12);
        }
    }
}
