using System;
using System.Runtime.InteropServices;
using System.Text;

static public partial class Yogi
{
    partial class Api
    {
        // === YOGI_BranchCreate ===
        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate int BranchCreateDelegate(ref IntPtr branch, SafeObjectHandle context,
            [MarshalAs(UnmanagedType.LPStr)] string name,
            [MarshalAs(UnmanagedType.LPStr)] string description,
            [MarshalAs(UnmanagedType.LPStr)] string netname,
            [MarshalAs(UnmanagedType.LPStr)] string password,
            [MarshalAs(UnmanagedType.LPStr)] string path,
            [MarshalAs(UnmanagedType.LPStr)] string advaddr, int advport, long advint,
            long timeout);

        public static BranchCreateDelegate YOGI_BranchCreate
            = Library.GetDelegateForFunction<BranchCreateDelegate>("YOGI_BranchCreate");

        // === YOGI_BranchGetInfo ===
        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate int BranchGetInfoDelegate(SafeObjectHandle branch,
            IntPtr uuid, [MarshalAs(UnmanagedType.LPStr)] StringBuilder json, int jsonsize);

        public static BranchGetInfoDelegate YOGI_BranchGetInfo
            = Library.GetDelegateForFunction<BranchGetInfoDelegate>("YOGI_BranchGetInfo");

        // === YOGI_BranchGetConnectedBranches ===
        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate void BranchGetConnectedBranchesFnDelegate(ErrorCode res, IntPtr userarg);

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate int BranchGetConnectedBranchesDelegate(SafeObjectHandle branch,
            IntPtr uuid, [MarshalAs(UnmanagedType.LPStr)] StringBuilder json, int jsonsize,
            BranchGetConnectedBranchesFnDelegate fn, IntPtr userarg);

        public static BranchGetConnectedBranchesDelegate YOGI_BranchGetConnectedBranches
            = Library.GetDelegateForFunction<BranchGetConnectedBranchesDelegate>(
                "YOGI_BranchGetConnectedBranches");

        // === YOGI_BranchAwaitEvent ===
        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate void BranchAwaitEventFnDelegate(ErrorCode res, BranchEvents evnt,
            ErrorCode evres, IntPtr userarg);

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate int BranchAwaitEventDelegate(SafeObjectHandle branch,
            BranchEvents events, IntPtr uuid, [MarshalAs(UnmanagedType.LPStr)] StringBuilder json,
            int jsonsize, BranchAwaitEventFnDelegate fn, IntPtr userarg);

        public static BranchAwaitEventDelegate YOGI_BranchAwaitEvent
            = Library.GetDelegateForFunction<BranchAwaitEventDelegate>("YOGI_BranchAwaitEvent");

        // === YOGI_BranchCancelAwaitEvent ===
        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate int BranchCancelAwaitEventDelegate(SafeObjectHandle branch);

        public static BranchCancelAwaitEventDelegate YOGI_BranchCancelAwaitEvent
            = Library.GetDelegateForFunction<BranchCancelAwaitEventDelegate>(
                "YOGI_BranchCancelAwaitEvent");
    }

    /// <summary>
    /// Events that can be observed on a branch.
    /// </summary>
    [Flags]
    public enum BranchEvents
    {
        /// <summary>No events.</summary>
        None = 0,

        /// <summary>A new branch has been discovered.</summary>
        BranchDiscovered = (1 << 0),

        /// <summary>Querying a new branch for information finished.</summary>
        BranchQueried = (1 << 1),

        /// <summary>Connecting to a branch finished.</summary>
        ConnectFinished = (1 << 2),

        /// <summary>The connection to a branch was lost.</summary>
        ConnectionLost = (1 << 3),

        /// <summary>Combination of all flags.</summary>
        All = BranchDiscovered | BranchQueried | ConnectFinished | ConnectionLost
    }

    public class Branch : Object
    {
        public Branch()
        : base(Create())
        {
        }

        static IntPtr Create()
        {
            var handle = new IntPtr();
            // int res = Api.YOGI_LoggerCreate(ref handle);
            // CheckErrorCode(res);
            return handle;
        }
    }
}
