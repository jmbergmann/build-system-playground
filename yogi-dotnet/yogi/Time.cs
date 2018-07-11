using System;
using System.Runtime.InteropServices;

static public partial class Yogi
{
    public static readonly TimeSpan InfiniteTimeSpan = TimeSpan.MaxValue;

    partial class Api
    {
        /// === YOGI_GetCurrentTime ===
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

    static long TimeSpanToCoreDuration(TimeSpan? duration)
    {
        if (!duration.HasValue) return 0;
        if (duration.Value == InfiniteTimeSpan) return -1;
        return duration.Value.Ticks * 100;
    }

    static TimeSpan DoubleToTimeSpan(double dur)
    {
        if (dur == -1) return InfiniteTimeSpan;
        return TimeSpan.FromSeconds(dur);
    }

    static DateTime StringToDateTime(string s)
    {
        return DateTime.Parse(s, null, System.Globalization.DateTimeStyles.RoundtripKind);
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
