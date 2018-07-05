using System;
using System.Runtime.InteropServices;

static public partial class Yogi
{
    partial class Api
    {
        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate int GetCurrentTimeDelegate(ref long timestamp);
        public static GetCurrentTimeDelegate YOGI_GetCurrentTime
            = Library.GetDelegateForFunction<GetCurrentTimeDelegate>("YOGI_GetCurrentTime");
    }

    static DateTime CoreTimestampToDateTime(long timestamp)
    {
        return new DateTime(timestamp / 100, DateTimeKind.Utc).AddYears(1969);
    }

    static TimeSpan CoreDurationToTimeSpan(long duration)
    {
        return new TimeSpan(duration / 100);
    }

    /// <summary>
    /// The current time.
    /// </summary>
    public static DateTime CurrentTime
    {
        get
        {
            long timestamp = -1;
            int res = Api.YOGI_GetCurrentTime(ref timestamp);
            CheckErrorCode(res);

            return CoreTimestampToDateTime(timestamp);
        }
    }
}
