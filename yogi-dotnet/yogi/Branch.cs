using System;
using System.Runtime.InteropServices;
using System.Text;
using Newtonsoft.Json;

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

    public class BranchInfo
    {
        public BranchInfo(string json)
        {
        }

        /// <summary>UUID of the branch.</summary>
        public Guid Uuid { get; }

        /// <summary>Name of the branch.</summary>
        public string Name { get; }

        /// <summary>Description of the branch.</summary>
        public string Description { get; }

        /// <summary>Name of the network.</summary>
        public string NetName { get; }

        /// <summary>Path of the branch.</summary>
        public string Path { get; }

        /// <summary>The machine's hostname..</summary>
        public string Hostname { get; }

        /// <summary>ID of the process.</summary>
        public int Pid { get; }

        /// <summary>Advertising interval.</summary>
        public TimeSpan AdvertisingInterval { get; }

        /// <summary>Address of the TCP server for incoming connections.</summary>
        public string TcpServerAddress { get; }

        /// <summary>Listening port of the TCP server for incoming connections.</summary>
        public int TcpServerPort { get; }

        /// <summary>Time when the branch was started.</summary>
        public DateTime StartTime { get; }

        /// <summary>Connection timeout.</summary>
        public TimeSpan Timeout { get; }
    }

    public class RemoteBranchInfo : BranchInfo
    {
        public RemoteBranchInfo(string json)
        : base(json)
        {
        }
    }

    public class LocalBranchInfo : BranchInfo
    {
        public LocalBranchInfo(string json)
        : base(json)
        {
        }

        /// <summary>Advertising IP address.</summary>
        public string AdvertisingAddress { get; }

        /// <summary>Advertising port.</summary>
        public int AdvertisingPort { get; }
    }

    public class BranchEventInfo
    {
        public BranchEventInfo(string json)
        {
        }

        /// <summary>UUID of the branch.</summary>
        public Guid Uuid { get; }
    }

    public class BranchDiscoveredEventInfo : BranchEventInfo
    {
        public BranchDiscoveredEventInfo(string json)
        : base(json)
        {
        }

        /// <summary>Address of the TCP server for incoming connections.</summary>
        public string TcpServerAddress { get; }

        /// <summary>Listening port of the TCP server for incoming connections.</summary>
        public int TcpServerPort { get; }
    }

    public class BranchQueriedEventInfo : BranchEventInfo
    {
        public BranchQueriedEventInfo(string json)
        : base(json)
        {
            info = new RemoteBranchInfo(json);
        }

        /// <summary>Name of the branch.</summary>
        public string Name { get { return info.Name; } }

        /// <summary>Description of the branch.</summary>
        public string Description { get { return info.Description; } }

        /// <summary>Name of the network.</summary>
        public string NetName { get { return info.NetName; } }

        /// <summary>Path of the branch.</summary>
        public string Path { get { return info.Path; } }

        /// <summary>The machine's hostname..</summary>
        public string Hostname { get { return info.Hostname; } }

        /// <summary>ID of the process.</summary>
        public int Pid { get { return info.Pid; } }

        /// <summary>Advertising interval.</summary>
        public TimeSpan AdvertisingInterval { get { return info.AdvertisingInterval; } }

        /// <summary>Address of the TCP server for incoming connections.</summary>
        public string TcpServerAddress { get { return info.TcpServerAddress; } }

        /// <summary>Listening port of the TCP server for incoming connections.</summary>
        public int TcpServerPort { get { return info.TcpServerPort; } }

        /// <summary>Time when the branch was started.</summary>
        public DateTime StartTime { get { return info.StartTime; } }

        /// <summary>Connection timeout.</summary>
        public TimeSpan Timeout { get { return info.Timeout; } }

        public RemoteBranchInfo ToRemoteBranchInfo()
        {
            return info;
        }

        RemoteBranchInfo info;
    }

    public class ConnectFinishedEventInfo : BranchEventInfo
    {
        public ConnectFinishedEventInfo(string json)
        : base(json)
        {
        }
    }

    public class ConnectionLostEventInfo : BranchEventInfo
    {
        public ConnectionLostEventInfo(string json)
        : base(json)
        {
        }
    }

    /// <summary>
    /// Entry point into a Yogi network.
    ///
    /// A branch represents an entry point into a YOGI network. It advertises
    /// itself via IP broadcasts/multicasts with its unique ID and information
    /// required for establishing a connection. If a branch detects other branches
    /// on the network, it connects to them via TCP to retrieve further
    /// information such as their name, description and network name. If the
    /// network names match, two branches attempt to authenticate with each other
    /// by securely comparing passwords. Once authentication succeeds and there is
    /// no other known branch with the same path then the branches can actively
    /// communicate as part of the Yogi network.
    ///
    /// Note: Even though the authentication process via passwords is done in a
    ///       secure manner, any further communication is done in plain text.
    /// </summary>
    public class Branch : Object
    {
        /// <summary>
        /// Creates the branch.
        ///
        /// Advertising and establishing connections can be limited to certain
        /// network interfaces via the interface parameter. The default is to use
        /// all available interfaces.
        ///
        /// Setting the advint parameter to infinity prevents the branch from
        /// actively participating in the Yogi network, i.e. the branch will not
        /// advertise itself and it will not authenticate in order to join a
        /// network. However, the branch will temporarily connect to other
        /// branches in order to obtain more detailed information such as name,
        /// description, network name and so on. This is useful for obtaining
        /// information about active branches without actually becoming part of
        /// the Yogi network.
        /// </summary>
        /// <param name="context">The context to use.</param>
        /// <param name="name">Name of the branch (by default, the format PID@hostname
        /// with PID being the process ID will be used).</param>
        /// <param name="description">Description of the branch.</param>
        /// <param name="netname">Name of the network to join (by default, the
        /// machine's hostname will be used as the network name).</param>
        /// <param name="password">Password for the network.</param>
        /// <param name="path">Path of the branch in the network (by default, the
        /// format /name where name is the branch's name will be used).</param>
        /// <param name="advaddr">Multicast address to use; e.g. 239.255.0.1 for IPv4
        /// or ff31::8000:1234 for IPv6.</param>
        /// <param name="advport">Advertising port.</param>
        /// <param name="advint">Advertising interval (must be at least 1 ms).</param>
        /// <param name="timeout">Maximum time of inactivity before a remote branch is
        /// considered to be dead (must be at least 1 ms).</param>
        /// <returns></returns>
        public Branch(Context context, string name, [Optional] string description,
            [Optional] string netname, [Optional] string password, [Optional] string path,
            [Optional] string advaddr, [Optional] int advport, [Optional] TimeSpan? advint,
            [Optional] TimeSpan? timeout)
        : base(Create(context, name, description, netname, password, path, advaddr, advport,
            advint, timeout), new Object[]{context})
        {
        }

        /// <summary>
        /// Information about the local branch.
        /// </summary>
        public LocalBranchInfo Info
        {
            get
            {
                if (info == null)
                {
                    var json = new StringBuilder(128);
                    int res;
                    do {
                        json = new StringBuilder(json.Capacity * 2);
                        res = Api.YOGI_BranchGetInfo(Handle, IntPtr.Zero, json, json.Capacity);
                    }
                    while (res == (int)ErrorCode.BufferTooSmall);
                    CheckErrorCode(res);
                    info = new LocalBranchInfo(json.ToString());
                }

                return info;
            }
        }

        static IntPtr Create(Context context, string name, [Optional] string description,
            [Optional] string netname, [Optional] string password, [Optional] string path,
            [Optional] string advaddr, [Optional] int advport, [Optional] TimeSpan? advint,
            [Optional] TimeSpan? timeout)
        {
            var handle = new IntPtr();
            int res = Api.YOGI_BranchCreate(ref handle, context.Handle, name, description, netname,
                password, path, advaddr, advport, TimeSpanToCoreDuration(advint),
                TimeSpanToCoreDuration(timeout));
            CheckErrorCode(res);
            return handle;
        }

        LocalBranchInfo info;
    }
}
