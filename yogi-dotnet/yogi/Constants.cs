using System;
using System.Runtime.InteropServices;

static public partial class Yogi
{
    partial class Api
    {
        // === YOGI_GetConstant ===
        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate int GetIntConstantDelegate(ref int dest, int constant);

        public static GetIntConstantDelegate YOGI_GetIntConstant
            = Library.GetDelegateForFunction<GetIntConstantDelegate>(
                "YOGI_GetConstant");

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate int GetLongLongConstantDelegate(ref long dest, int constant);

        public static GetLongLongConstantDelegate YOGI_GetLongLongConstant
            = Library.GetDelegateForFunction<GetLongLongConstantDelegate>(
                "YOGI_GetConstant");

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate int GetStringConstantDelegate(ref IntPtr dest, int constant);

        public static GetStringConstantDelegate YOGI_GetStringConstant
            = Library.GetDelegateForFunction<GetStringConstantDelegate>(
                "YOGI_GetConstant");
    }

    /// <summary>
    /// Constants built into the Yogi Core library.
    /// </summary>
    static public class Constants
    {
        /// <summary>Complete Yogi Core version number.</summary>
        static public readonly string VersionNumber;

        /// <summary>Yogi Core major version number.</summary>
        static public readonly int VersionMajor;

        /// <summary>Yogi Core minor version number.</summary>
        static public readonly int VersionMinor;

        /// <summary>Yogi Core patch version number.</summary>
        static public readonly int VersionPatch;

        /// <summary>Default IP address for advertising.</summary>
        static public readonly string DefaultAdvAddress;

        /// <summary>Default UDP port for advertising.</summary>
        static public readonly int DefaultAdvPort;

        /// <summary>Default time between two advertising messages.</summary>
        static public readonly TimeSpan DefaultAdvInterval;

        /// <summary>Default timeout for connections between two branches.</summary>
        static public readonly TimeSpan DefaultConnectionTimeout;

        /// <summary>Default verbosity for newly created loggers.</summary>
        static public readonly Verbosity DefaultLoggerVerbosity;

        /// <summary>Default format of the time string in log entries.</summary>
        static public readonly string DefaultLogTimeFormat;

        /// <summary>Default format of a log entry.</summary>
        static public readonly string DefaultLogFormat;

        /// <summary>Maximum size of a message between two branches.</summary>
        static public readonly int MaxMessageSize;

        static Constants()
        {
            // TODO: Handle return value
            IntPtr str = new IntPtr();
            CheckErrorCode(Api.YOGI_GetStringConstant(ref str, 1));
            VersionNumber = Marshal.PtrToStringAnsi(str);

            CheckErrorCode(Api.YOGI_GetIntConstant(ref VersionMajor, 2));
            CheckErrorCode(Api.YOGI_GetIntConstant(ref VersionMinor, 3));
            CheckErrorCode(Api.YOGI_GetIntConstant(ref VersionPatch, 4));

            CheckErrorCode(Api.YOGI_GetStringConstant(ref str, 5));
            DefaultAdvAddress = Marshal.PtrToStringAnsi(str);

            CheckErrorCode(Api.YOGI_GetIntConstant(ref DefaultAdvPort, 6));

            long t = -1;
            CheckErrorCode(Api.YOGI_GetLongLongConstant(ref t, 7));
            DefaultAdvInterval = CoreDurationToTimeSpan(t);

            CheckErrorCode(Api.YOGI_GetLongLongConstant(ref t, 8));
            DefaultConnectionTimeout = CoreDurationToTimeSpan(t);

            int n = -1;
            CheckErrorCode(Api.YOGI_GetIntConstant(ref n, 9));
            DefaultLoggerVerbosity = (Verbosity)n;

            CheckErrorCode(Api.YOGI_GetStringConstant(ref str, 10));
            DefaultLogTimeFormat = Marshal.PtrToStringAnsi(str);

            CheckErrorCode(Api.YOGI_GetStringConstant(ref str, 11));
            DefaultLogFormat = Marshal.PtrToStringAnsi(str);

            CheckErrorCode(Api.YOGI_GetIntConstant(ref MaxMessageSize, 12));
        }
    }
}
