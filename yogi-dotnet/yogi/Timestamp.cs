// using System;
// using System.Runtime.InteropServices;
// using System.Text;

// static public partial class Yogi
// {
//     partial class Api
//     {
//         // === YOGI_GetCurrentTime ===
//         [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
//         public delegate int GetCurrentTimeDelegate(ref long timestamp);

//         public static GetCurrentTimeDelegate YOGI_GetCurrentTime
//             = Library.GetDelegateForFunction<GetCurrentTimeDelegate>(
//                 "YOGI_GetCurrentTime");

//         // === YOGI_FormatTime ===
//         [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
//         public delegate int FormatTimeDelegate(long timestamp,
//             [MarshalAs(UnmanagedType.LPStr)] StringBuilder str, int strsize,
//             [MarshalAs(UnmanagedType.LPStr)] string timefmt);

//         public static FormatTimeDelegate YOGI_FormatTime
//             = Library.GetDelegateForFunction<FormatTimeDelegate>(
//                 "YOGI_FormatTime");

//         // === YOGI_ParseTime ===
//         [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
//         public delegate int ParseTimeDelegate(ref long timestamp,
//             [MarshalAs(UnmanagedType.LPStr)] string str,
//             [MarshalAs(UnmanagedType.LPStr)] string timefmt);

//         public static ParseTimeDelegate YOGI_ParseTime
//             = Library.GetDelegateForFunction<ParseTimeDelegate>(
//                 "YOGI_ParseTime");
//     }

//     public class Timestamp
//     {
//         static public Timestamp Now()
//         {

//         }

//         static public Timestamp Parse(string str, [Optional] string timefmt)
//         {

//         }

//         public Timestamp() : this(0)
//         {
//         }

//         public Timestamp(long nsSinceEpoch)
//         {
//             NanosecondsSinceEpoch = nsSinceEpoch;
//         }

//         public Timestamp(TimeSpan dur) : this (dur.Ticks * 100)
//         {
//         }

//         public long NanosecondsSinceEpoch { get; private set; }

//         public int Nanosecond { get { return (int)(NanosecondsSinceEpoch % 1000); } }

//         public int Microsecond { get { return (int)((NanosecondsSinceEpoch / 1000) % 1000); } }

//         public int Millisecond { get { return (int)((NanosecondsSinceEpoch / 1000000) % 1000); } }

//         public string Format([Optional] string timefmt)
//         {

//         }

//         public override string ToString()
//         {
//             return Format();
//         }

//         public static Timestamp operator+ (Timestamp timestamp, TimeSpan dur)
//         {
//             return new Timestamp(timestamp.NanosecondsSinceEpoch + rhs);
//         }
//     }
// }
