/*
 * This file is part of the Yogi distribution https://github.com/yohummus/yogi.
 * Copyright (c) 2018 Johannes Bergmann.
 *
 * This library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library. If not, see <http://www.gnu.org/licenses/>.
 */

using System;
using System.Runtime.InteropServices;
using System.Text;
using System.Net;
using System.IO;
using System.Collections.Generic;
using Newtonsoft.Json;
using Newtonsoft.Json.Linq;

public static partial class Yogi
{
    partial class Api
    {
        // === YOGI_BranchCreate ===
        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate int BranchCreateDelegate(ref IntPtr branch, SafeObjectHandle context,
            [MarshalAs(UnmanagedType.LPStr)] string props,
            [MarshalAs(UnmanagedType.LPStr)] string section,
            [MarshalAs(UnmanagedType.LPStr)] StringBuilder err, int errsize);

        public static BranchCreateDelegate YOGI_BranchCreate
            = Library.GetDelegateForFunction<BranchCreateDelegate>(
                "YOGI_BranchCreate");

        // === YOGI_BranchGetInfo ===
        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate int BranchGetInfoDelegate(SafeObjectHandle branch,
            IntPtr uuid, [MarshalAs(UnmanagedType.LPStr)] StringBuilder json, int jsonsize);

        public static BranchGetInfoDelegate YOGI_BranchGetInfo
            = Library.GetDelegateForFunction<BranchGetInfoDelegate>(
                "YOGI_BranchGetInfo");

        // === YOGI_BranchGetConnectedBranches ===
        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate void BranchGetConnectedBranchesFnDelegate(ErrorCode res, IntPtr userarg);

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate int BranchGetConnectedBranchesDelegate(SafeObjectHandle branch,
            IntPtr uuid, IntPtr json, int jsonsize, BranchGetConnectedBranchesFnDelegate fn,
            IntPtr userarg);

        public static BranchGetConnectedBranchesDelegate YOGI_BranchGetConnectedBranches
            = Library.GetDelegateForFunction<BranchGetConnectedBranchesDelegate>(
                "YOGI_BranchGetConnectedBranches");

        // === YOGI_BranchAwaitEvent ===
        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate void BranchAwaitEventFnDelegate(int res, BranchEvents ev, int evres,
                                                        IntPtr userarg);

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate int BranchAwaitEventDelegate(SafeObjectHandle branch,
            BranchEvents events, IntPtr uuid, IntPtr json, int jsonsize,
            BranchAwaitEventFnDelegate fn, IntPtr userarg);

        public static BranchAwaitEventDelegate YOGI_BranchAwaitEvent
            = Library.GetDelegateForFunction<BranchAwaitEventDelegate>(
                "YOGI_BranchAwaitEvent");

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

    /// <summary>
    /// Information about about a branch.
    /// </summary>
    public class BranchInfo
    {
        /// <summary>
        /// Constructor.
        /// </summary>
        /// <param name="json">JSON string to parse.</param>
        public BranchInfo(string json)
        {
            this.json = json;

            JsonReader reader = new JsonTextReader(new StringReader(json));
            reader.DateParseHandling = DateParseHandling.None;
            Data = JObject.Load(reader);

            Uuid = Guid.Parse((string)Data["uuid"]);
            Name = (string)Data["name"];
            Description = (string)Data["description"];
            NetworkName = (string)Data["network_name"];
            Path = (string)Data["path"];
            Hostname = (string)Data["hostname"];
            Pid = (int)Data["pid"];
            AdvertisingInterval = Duration.FromJson(Data["advertising_interval"]);
            TcpServerAddress = IPAddress.Parse((string)Data["tcp_server_address"]);
            TcpServerPort = (int)Data["tcp_server_port"];
            StartTime = Timestamp.Parse((string)Data["start_time"]);
            Timeout = Duration.FromJson(Data["timeout"]);
            GhostMode = (bool)Data["ghost_mode"];
        }

        public override string ToString()
        {
            return json;
        }

        string json;

        protected JObject Data { get; }

        /// <summary>UUID of the branch.</summary>
        public Guid Uuid { get; }

        /// <summary>Name of the branch.</summary>
        public string Name { get; }

        /// <summary>Description of the branch.</summary>
        public string Description { get; }

        /// <summary>Name of the network.</summary>
        public string NetworkName { get; }

        /// <summary>Path of the branch.</summary>
        public string Path { get; }

        /// <summary>The machine's hostname.</summary>
        public string Hostname { get; }

        /// <summary>ID of the process.</summary>
        public int Pid { get; }

        /// <summary>Advertising interval.</summary>
        public Duration AdvertisingInterval { get; }

        /// <summary>Address of the TCP server for incoming connections.</summary>
        public IPAddress TcpServerAddress { get; }

        /// <summary>Listening port of the TCP server for incoming connections.</summary>
        public int TcpServerPort { get; }

        /// <summary>Time when the branch was started.</summary>
        public Timestamp StartTime { get; }

        /// <summary>Connection timeout.</summary>
        public Duration Timeout { get; }

        /// <summary>True if the branch is in ghost mode.</summary>
        public bool GhostMode { get; }
    }

    /// <summary>
    /// Information about a remote branch.
    /// </summary>
    public class RemoteBranchInfo : BranchInfo
    {
        /// <summary>
        /// Constructor.
        /// </summary>
        /// <param name="json">JSON string to parse.</param>
        public RemoteBranchInfo(string json)
        : base(json)
        {
        }
    }

    /// <summary>
    /// Information about a local branch.
    /// </summary>
    public class LocalBranchInfo : BranchInfo
    {
        /// <summary>
        /// Constructor.
        /// </summary>
        /// <param name="json">JSON string to parse.</param>
        public LocalBranchInfo(string json)
        : base(json)
        {
            AdvertisingAddress = IPAddress.Parse((string)Data["advertising_address"]);
            AdvertisingPort = (int)Data["advertising_port"];
            TxQueueSize = (int)Data["tx_queue_size"];
            RxQueueSize = (int)Data["rx_queue_size"];
        }

        /// <summary>Advertising IP address.</summary>
        public IPAddress AdvertisingAddress { get; }

        /// <summary>Advertising port.</summary>
        public int AdvertisingPort { get; }

        /// <summary>Size of the send queue for remote branches.</summary>
        public int TxQueueSize { get; }

        /// <summary>Size of the receive queue for remote branches.</summary>
        public int RxQueueSize { get; }
    }

    /// <summary>
    /// Information associated with a branch event.
    /// </summary>
    public class BranchEventInfo
    {
        /// <summary>
        /// Constructor.
        /// </summary>
        /// <param name="json">JSON string to parse.</param>
        public BranchEventInfo(string json)
        {
            this.json = json;
            Data = JObject.Parse(json);

            Uuid = Guid.Parse((string)Data["uuid"]);
        }

        public override string ToString()
        {
            return json;
        }

        string json;

        protected JObject Data { get; }

        /// <summary>UUID of the branch.</summary>
        public Guid Uuid { get; }
    }

    /// <summary>
    /// Information associated with the BranchDiscovered branch event.
    /// </summary>
    public class BranchDiscoveredEventInfo : BranchEventInfo
    {
        /// <summary>
        /// Constructor.
        /// </summary>
        /// <param name="json">JSON string to parse.</param>
        public BranchDiscoveredEventInfo(string json)
        : base(json)
        {
            TcpServerAddress = IPAddress.Parse((string)Data["tcp_server_address"]);
            TcpServerPort = (int)Data["tcp_server_port"];
        }

        /// <summary>Address of the TCP server for incoming connections.</summary>
        public IPAddress TcpServerAddress { get; }

        /// <summary>Listening port of the TCP server for incoming connections.</summary>
        public int TcpServerPort { get; }
    }

    /// <summary>
    /// Information associated with the BranchQueried branch event.
    /// </summary>
    public class BranchQueriedEventInfo : BranchEventInfo
    {
        /// <summary>
        /// Constructor.
        /// </summary>
        /// <param name="json">JSON string to parse.</param>
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
        public string NetName { get { return info.NetworkName; } }

        /// <summary>Path of the branch.</summary>
        public string Path { get { return info.Path; } }

        /// <summary>The machine's hostname..</summary>
        public string Hostname { get { return info.Hostname; } }

        /// <summary>ID of the process.</summary>
        public int Pid { get { return info.Pid; } }

        /// <summary>Advertising interval.</summary>
        public Duration AdvertisingInterval { get { return info.AdvertisingInterval; } }

        /// <summary>Address of the TCP server for incoming connections.</summary>
        public IPAddress TcpServerAddress { get { return info.TcpServerAddress; } }

        /// <summary>Listening port of the TCP server for incoming connections.</summary>
        public int TcpServerPort { get { return info.TcpServerPort; } }

        /// <summary>Time when the branch was started.</summary>
        public Timestamp StartTime { get { return info.StartTime; } }

        /// <summary>Connection timeout.</summary>
        public Duration Timeout { get { return info.Timeout; } }

        /// <summary>True if the branch is in ghost mode.</summary>
        public bool GhostMode { get { return info.GhostMode; } }

        /// <summary>
        /// Converts the event information to a RemoteBranchInfo object.
        /// </summary>
        /// <returns>Converted RemoteBranchInfo object.</returns>
        public RemoteBranchInfo ToRemoteBranchInfo()
        {
            return info;
        }

        RemoteBranchInfo info;
    }

    /// <summary>
    /// Information associated with the ConnectFinished branch event.
    /// </summary>
    public class ConnectFinishedEventInfo : BranchEventInfo
    {
        /// <summary>
        /// Constructor.
        /// </summary>
        /// <param name="json">JSON string to parse.</param>
        public ConnectFinishedEventInfo(string json)
        : base(json)
        {
        }
    }

    /// <summary>
    /// Information associated with the ConnectionLost branch event.
    /// </summary>
    public class ConnectionLostEventInfo : BranchEventInfo
    {
        /// <summary>
        /// Constructor.
        /// </summary>
        /// <param name="json">JSON string to parse.</param>
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
        /// The branch is configured via the props parameter. The supplied JSON must
        /// have the following structure:
        ///
        ///    {
        ///      "name":                 "Fan Controller",
        ///      "description":          "Controls a fan via PWM",
        ///      "path":                 "/Cooling System/Fan Controller",
        ///      "network_name":         "Hardware Control",
        ///      "network_password":     "secret",
        ///      "advertising_address":  "ff31::8000:2439",
        ///      "advertising_port":     13531,
        ///      "advertising_interval": 1.0,
        ///      "timeout":              3.0,
        ///      "ghost_mode":           false
        ///    }
        ///
        /// All of the properties are optional and if unspecified (or set to null),
        /// their respective default values will be used. The properties have the
        /// following meaning:
        ///  - name: Name of the branch (default: PID@hostname).
        ///  - description: Description of the branch.
        ///  - path: Path of the branch in the network (default: /name where name is
        ///    the name of the branch). Must start with a slash.
        ///  - network_name: Name of the network to join (default: the machine's
        ///    hostname).
        ///  - network_password: Password for the network (default: no password)
        ///  - advertising_address: Multicast address to use for advertising, e.g.
        ///    239.255.0.1 for IPv4 or ff31::8000:1234 for IPv6.
        ///  - advertising_port: Port to use for advertising.
        ///  - advertising_interval: Time between advertising messages. Must be at
        ///    least 1 ms.
        ///  - ghost_mode: Set to true to activate ghost mode (default: false).
        ///  - tx_queue_size: Size of the send queues for remote branches.
        ///  - rx_queue_size: Size of the receive queues for remote branches.
        ///
        /// Advertising and establishing connections can be limited to certain network
        /// interfaces via the _interface_ property. The default is to use all
        /// available interfaces.
        ///
        /// Setting the ghost_mode property to true prevents the branch from actively
        /// participating in the Yogi network, i.e. the branch will not advertise itself
        /// and it will not authenticate in order to join a network. However, the branch
        /// will temporarily connect to other branches in order to obtain more detailed
        /// information such as name, description, network name and so on. This is useful
        /// for obtaining information about active branches without actually becoming
        /// part of the Yogi network.
        ///
        /// Attention:
        ///   The tx_queue_size and rx_queue_size properties affect every branch
        ///   connection and can therefore consume a large amount of memory. For
        ///   example, in a network of 10 branches where these properties are set to 1 MB,
        ///   the resulting memory used for the queues would be 10 x 2 x 1 MB = 20 MB
        ///   for each of the 10 branches. This value grows with the number of branches squared.
        /// </summary>
        /// <param name="context">The context to use.</param>
        /// <param name="props">Branch properties as serialized JSON.</param>
        /// <param name="section">Section in props to use instead of the root section.
        /// Syntax is JSON pointer (RFC 6901).</param>
        public Branch(Context context, [Optional] string props, [Optional] string section)
        : base(Create(context, props, section), new Object[] { context })
        {
            Info = GetInfo();
        }

        /// <summary>
        /// Creates the branch.
        ///
        /// The branch is configured via the props parameter. The supplied JSON must
        /// have the following structure:
        ///
        ///    {
        ///      "name":                 "Fan Controller",
        ///      "description":          "Controls a fan via PWM",
        ///      "path":                 "/Cooling System/Fan Controller",
        ///      "network_name":         "Hardware Control",
        ///      "network_password":     "secret",
        ///      "advertising_address":  "ff31::8000:2439",
        ///      "advertising_port":     13531,
        ///      "advertising_interval": 1.0,
        ///      "timeout":              3.0,
        ///      "ghost_mode":           false,
        ///      "tx_queue_size":        1000000,
        ///      "rx_queue_size":        100000
        ///    }
        ///
        /// All of the properties are optional and if unspecified (or set to null),
        /// their respective default values will be used. The properties have the
        /// following meaning:
        ///  - name: Name of the branch (default: PID@hostname).
        ///  - description: Description of the branch.
        ///  - path: Path of the branch in the network (default: /name where name is
        ///    the name of the branch). Must start with a slash.
        ///  - network_name: Name of the network to join (default: the machine's
        ///    hostname).
        ///  - network_password: Password for the network (default: no password)
        ///  - advertising_address: Multicast address to use for advertising, e.g.
        ///    239.255.0.1 for IPv4 or ff31::8000:1234 for IPv6.
        ///  - advertising_port: Port to use for advertising.
        ///  - advertising_interval: Time between advertising messages. Must be at
        ///    least 1 ms.
        ///  - ghost_mode: Set to true to activate ghost mode (default: false).
        ///  - tx_queue_size: Size of the send queues for remote branches.
        ///  - rx_queue_size: Size of the receive queues for remote branches.
        ///
        /// Advertising and establishing connections can be limited to certain network
        /// interfaces via the _interface_ property. The default is to use all
        /// available interfaces.
        ///
        /// Setting the ghost_mode property to true prevents the branch from actively
        /// participating in the Yogi network, i.e. the branch will not advertise itself
        /// and it will not authenticate in order to join a network. However, the branch
        /// will temporarily connect to other branches in order to obtain more detailed
        /// information such as name, description, network name and so on. This is useful
        /// for obtaining information about active branches without actually becoming
        /// part of the Yogi network.
        /// </summary>
        /// <param name="context">The context to use.</param>
        /// <param name="props">Branch properties.</param>
        public Branch(Context context, JObject props)
        : this(context, JsonConvert.SerializeObject(props))
        {
        }

        /// <summary>Information about the local branch.</summary>
        public LocalBranchInfo Info { get; }

        /// <summary>UUID of the branch.</summary>
        public Guid Uuid { get { return Info.Uuid; } }

        /// <summary>Name of the branch.</summary>
        public string Name { get { return Info.Name; } }

        /// <summary>Description of the branch.</summary>
        public string Description { get { return Info.Description; } }

        /// <summary>Name of the network.</summary>
        public string NetName { get { return Info.NetworkName; } }

        /// <summary>Path of the branch.</summary>
        public string Path { get { return Info.Path; } }

        /// <summary>The machine's hostname..</summary>
        public string Hostname { get { return Info.Hostname; } }

        /// <summary>ID of the process.</summary>
        public int Pid { get { return Info.Pid; } }

        /// <summary>Advertising interval.</summary>
        public Duration AdvertisingInterval { get { return Info.AdvertisingInterval; } }

        /// <summary>Address of the TCP server for incoming connections.</summary>
        public IPAddress TcpServerAddress { get { return Info.TcpServerAddress; } }

        /// <summary>Listening port of the TCP server for incoming connections.</summary>
        public int TcpServerPort { get { return Info.TcpServerPort; } }

        /// <summary>Time when the branch was started.</summary>
        public Timestamp StartTime { get { return Info.StartTime; } }

        /// <summary>Connection timeout.</summary>
        public Duration Timeout { get { return Info.Timeout; } }

        /// <summary>True if the branch is in ghost mode.</summary>
        public bool GhostMode { get { return Info.GhostMode; } }

        /// <summary>Advertising IP address.</summary>
        public IPAddress AdvertisingAddress { get { return Info.AdvertisingAddress; } }

        /// <summary>Advertising port.</summary>
        public int AdvertisingPort { get { return Info.AdvertisingPort; } }

        /// <summary>Size of the send queue for remote branches.</summary>
        public int TxQueueSize { get { return Info.TxQueueSize; } }

        /// <summary>Size of the receive queue for remote branches.</summary>
        public int RxQueueSize { get { return Info.RxQueueSize; } }

        /// <summary>
        /// Retrieves information about all connected remote branches.
        /// </summary>
        /// <returns>Dictionary mapping the UUID of each connected remote branch to a
        /// RemoteBranchInfo object with detailed information about the branch.</returns>
        public Dictionary<Guid, RemoteBranchInfo> GetConnectedBranches()
        {
            var branches = new Dictionary<Guid, RemoteBranchInfo>();
            var size = 1024;

            int res;
            do
            {
                branches.Clear();
                var json = Marshal.AllocHGlobal(size);
                size *= 2;

                Api.BranchGetConnectedBranchesFnDelegate fn = (ec, userarg) =>
                {
                    if (ec != ErrorCode.Ok) return;
                    var info = new RemoteBranchInfo(Marshal.PtrToStringAnsi(json));
                    branches.Add(info.Uuid, info);
                };

                res = Api.YOGI_BranchGetConnectedBranches(Handle, IntPtr.Zero, json, size,
                                                          fn, IntPtr.Zero);
                Marshal.FreeHGlobal(json);
            }
            while (res == (int)ErrorCode.BufferTooSmall);
            CheckErrorCode(res);

            return branches;
        }

        /// <summary>
        /// Delegate for the await event handler function.
        /// </summary>
        /// <param name="res">Result of the wait operation.</param>
        /// <param name="ev">The event that occurred.</param>
        /// <param name="evres">Result associated with the event.</param>
        /// <param name="info">Event information.</param>
        public delegate void AwaitEventFnDelegate(Result res, BranchEvents ev, Result evres,
                                                  [Optional] BranchEventInfo info);

        /// <summary>
        /// Waits for a branch event to occur.
        ///
        /// This function will register the handler fn to be executed once one of the given branch
        /// events occurs. If this function is called while a previous wait operation is still
        /// active then the previous opertion will be canceled, i.e. the handler fn for the
        /// previous operation will be called with a cancellation error.
        ///
        /// If successful, the event information passed to the handler function fn contains at
        /// least the UUID of the remote branch.
        ///
        /// In case that the internal buffer for reading the event information is too small, fn
        /// will be called with the corresponding error and the event information is lost. You can
        /// set the size of this buffer via the bufferSize parameter.
        /// </summary>
        /// <param name="events">Events to observe.</param>
        /// <param name="fn">Handler function to call.</param>
        /// <param name="bufferSize">Size of the internal buffer for reading the event
        /// information.</param>
        public void AwaitEvent(BranchEvents events, AwaitEventFnDelegate fn, int bufferSize = 1024)
        {
            var json = Marshal.AllocHGlobal(bufferSize);

            Api.BranchAwaitEventFnDelegate wrapper = (res, ev, evres, userarg) =>
            {
                var result = ErrorCodeToResult(res);
                BranchEventInfo info = null;
                if (result)
                {
                    var jsonStr = Marshal.PtrToStringAnsi(json);
                    Marshal.FreeHGlobal(json);

                    switch (ev)
                    {
                        case BranchEvents.BranchDiscovered:
                            info = new BranchDiscoveredEventInfo(jsonStr);
                            break;

                        case BranchEvents.BranchQueried:
                            info = new BranchQueriedEventInfo(jsonStr);
                            break;

                        case BranchEvents.ConnectFinished:
                            info = new ConnectFinishedEventInfo(jsonStr);
                            break;

                        case BranchEvents.ConnectionLost:
                            info = new ConnectionLostEventInfo(jsonStr);
                            break;
                    }
                }

                try
                {
                    fn(result, ev, ErrorCodeToResult(evres), info);
                }
                finally
                {
                    GCHandle.FromIntPtr(userarg).Free();
                }
            };
            var wrapperHandle = GCHandle.Alloc(wrapper);

            try
            {
                var wrapperPtr = GCHandle.ToIntPtr(wrapperHandle);
                int res = Api.YOGI_BranchAwaitEvent(Handle, events, IntPtr.Zero, json,
                                                    bufferSize, wrapper, wrapperPtr);
                CheckErrorCode(res);
            }
            catch
            {
                wrapperHandle.Free();
                Marshal.FreeHGlobal(json);
                throw;
            }
        }

        /// <summary>
        /// Cancels waiting for a branch event.
        ///
        /// Calling this function will cause the handler registered via AwaitEvent() to be called
        /// with a cancellation error.
        /// </summary>
        public void CancelAwaitEvent()
        {
            int res = Api.YOGI_BranchCancelAwaitEvent(Handle);
            CheckErrorCode(res);
        }

        static IntPtr Create(Context context, [Optional] string props, [Optional] string section)
        {
            var handle = new IntPtr();
            CheckDescriptiveErrorCode((err) =>
            {
                return Api.YOGI_BranchCreate(ref handle, context.Handle, props, section, err, err.Capacity);
            });
            return handle;
        }

        LocalBranchInfo GetInfo()
        {
            StringBuilder json;
            var size = 1024;
            int res;
            do
            {
                json = new StringBuilder(size);
                size *= 2;
                res = Api.YOGI_BranchGetInfo(Handle, IntPtr.Zero, json, json.Capacity);
            }
            while (res == (int)ErrorCode.BufferTooSmall);
            CheckErrorCode(res);
            return new LocalBranchInfo(json.ToString());
        }
    }
}
