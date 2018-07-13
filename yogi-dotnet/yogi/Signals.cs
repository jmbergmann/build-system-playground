using System;
using System.Runtime.InteropServices;

static public partial class Yogi
{
    partial class Api
    {
        // === YOGI_RaiseSignal ===
        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate void RaiseSignalFnDelegate(IntPtr sigarg, IntPtr userarg);

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate int RaiseSignalDelegate(Signals signal, IntPtr sigarg,
            RaiseSignalFnDelegate fn, IntPtr userarg);

        public static RaiseSignalDelegate YOGI_RaiseSignal
            = Library.GetDelegateForFunction<RaiseSignalDelegate>(
                "YOGI_RaiseSignal");

        // === YOGI_SignalSetCreate ===
        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate int SignalSetCreateDelegate(ref IntPtr sigset,
            SafeObjectHandle context, Signals signals);

        public static SignalSetCreateDelegate YOGI_SignalSetCreate
            = Library.GetDelegateForFunction<SignalSetCreateDelegate>(
                "YOGI_SignalSetCreate");

        // === YOGI_SignalSetAwaitSignal ===
        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate void SignalSetAwaitSignalFnDelegate(int res, Signals signal,
            IntPtr sigarg, IntPtr userarg);

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate int SignalSetAwaitSignalDelegate(SafeObjectHandle timer,
            long duration, SignalSetAwaitSignalFnDelegate fn, IntPtr userarg);

        public static SignalSetAwaitSignalDelegate YOGI_SignalSetAwaitSignal
            = Library.GetDelegateForFunction<SignalSetAwaitSignalDelegate>(
                "YOGI_SignalSetAwaitSignal");

        // === YOGI_SignalSetCancelAwaitSignal ===
        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate int SignalSetCancelAwaitSignalDelegate(SafeObjectHandle sigset);

        public static SignalSetCancelAwaitSignalDelegate YOGI_SignalSetCancelAwaitSignal
            = Library.GetDelegateForFunction<SignalSetCancelAwaitSignalDelegate>(
                "YOGI_SignalSetCancelAwaitSignal");
    }

    /// <summary>
    /// Signals.
    /// </summary>
    [Flags]
    public enum Signals
    {
        /// <summary>No signal.</summary>
        None = 0,

        /// <summary>Interrupt (e.g. by receiving SIGINT or pressing STRG + C).</summary>
        Int = (1 << 0),

        /// <summary>Termination request (e.g. by receiving SIGTERM).</summary>
        Term = (1 << 1),

        /// <summary>User-defined signal 1.</summary>
        Usr1 = (1 << 24),

        /// <summary>User-defined signal 2.</summary>
        Usr2 = (1 << 25),

        /// <summary>User-defined signal 3.</summary>
        Usr3 = (1 << 26),

        /// <summary>User-defined signal 4.</summary>
        Usr4 = (1 << 27),

        /// <summary>User-defined signal 5.</summary>
        Usr5 = (1 << 28),

        /// <summary>User-defined signal 6.</summary>
        Usr6 = (1 << 29),

        /// <summary>User-defined signal 7.</summary>
        Usr7 = (1 << 30),

        /// <summary>User-defined signal 8.</summary>
        Usr8 = (1 << 31),

        /// <summary>Combination of all flags.</summary>
        All = Int | Term | Usr1 | Usr2 | Usr3 | Usr4 | Usr5 | Usr6 | Usr7 | Usr8
    }
}
