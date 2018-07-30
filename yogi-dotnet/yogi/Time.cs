using System;
using System.Runtime.InteropServices;
using System.Text;

static public partial class Yogi
{
    public static readonly TimeSpan InfiniteTimeSpan = TimeSpan.MaxValue;

    partial class Api
    {
        // === YOGI_GetCurrentTime ===
        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate int GetCurrentTimeDelegate(ref long timestamp);

        public static GetCurrentTimeDelegate YOGI_GetCurrentTime
            = Library.GetDelegateForFunction<GetCurrentTimeDelegate>(
                "YOGI_GetCurrentTime");

        // === YOGI_FormatTime ===
        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate int FormatTimeDelegate(long timestamp,
            [MarshalAs(UnmanagedType.LPStr)] StringBuilder str, int strsize,
            [MarshalAs(UnmanagedType.LPStr)] string timefmt);

        public static FormatTimeDelegate YOGI_FormatTime
            = Library.GetDelegateForFunction<FormatTimeDelegate>(
                "YOGI_FormatTime");

        // === YOGI_ParseTime ===
        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate int ParseTimeDelegate(ref long timestamp,
            [MarshalAs(UnmanagedType.LPStr)] string str,
            [MarshalAs(UnmanagedType.LPStr)] string timefmt);

        public static ParseTimeDelegate YOGI_ParseTime
            = Library.GetDelegateForFunction<ParseTimeDelegate>(
                "YOGI_ParseTime");
    }

    static DateTime CoreTimestampToDateTime(long timestamp)
    {
        return new DateTime(timestamp / 100, DateTimeKind.Utc).AddYears(1969);
    }

    static long DateTimeToCoreTimestamp(DateTime time)
    {
        return time.ToUniversalTime().AddYears(-1969).Ticks * 100;
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

    /// <summary>
    /// Converts a timestamp into a string.
    ///
    /// The timefmt parameter describes the textual format of the conversion. The
    /// following placeholders are supported:
    ///     %Y: Four digit year.
    ///     %m: Month name as a decimal 01 to 12.
    ///     %d: Day of the month as decimal 01 to 31.
    ///     %F: Equivalent to %Y-%m-%d (the ISO 8601 date format).
    ///     %H: The hour as a decimal number using a 24-hour clock (00 to 23).
    ///     %M: The minute as a decimal 00 to 59.
    ///     %S: Seconds as a decimal 00 to 59.
    ///     %T: Equivalent to %H:%M:%S (the ISO 8601 time format).
    ///     %3: Milliseconds as decimal number 000 to 999.
    ///     %6: Microseconds as decimal number 000 to 999.
    ///     %9: Nanoseconds as decimal number 000 to 999.
    ///
    /// By default, the timestamp will be formatted in the format
    /// "2009-02-11T12:53:09.123Z".
    /// </summary>
    /// <param name="time">The timestamp.</param>
    /// <param name="fmt">Format of the time string.</param>
    /// <returns>The timestamp as a string.</returns>
    public static string FormatTime(DateTime time, [Optional] string fmt)
    {
        var str = new StringBuilder(128);
        int res = Api.YOGI_FormatTime(DateTimeToCoreTimestamp(time), str, str.Capacity,
            fmt);
        CheckErrorCode(res);
        return str.ToString();
    }

    /// <summary>
    /// Converts a string into a timestamp.
    ///
    /// The timefmt parameter describes the textual format of the conversion. The
    /// following placeholders are supported:
    ///     %Y: Four digit year.
    ///     %m: Month name as a decimal 01 to 12.
    ///     %d: Day of the month as decimal 01 to 31.
    ///     %F: Equivalent to %Y-%m-%d (the ISO 8601 date format).
    ///     %H: The hour as a decimal number using a 24-hour clock (00 to 23).
    ///     %M: The minute as a decimal 00 to 59.
    ///     %S: Seconds as a decimal 00 to 59.
    ///     %T: Equivalent to %H:%M:%S (the ISO 8601 time format).
    ///     %3: Milliseconds as decimal number 000 to 999.
    ///     %6: Microseconds as decimal number 000 to 999.
    ///     %9: Nanoseconds as decimal number 000 to 999.
    ///
    /// By default, the timestamp will be parsed in the format
    /// "2009-02-11T12:53:09.123Z".
    /// </summary>
    /// <param name="s">String to parse.</param>
    /// <param name="fmt">Format of the time string.</param>
    /// <returns>The timestamp.</returns>
    public static DateTime ParseTime(string s, [Optional] string fmt)
    {
        long timestamp = -1;
        int res = Api.YOGI_ParseTime(ref timestamp, s, fmt);
        return CoreTimestampToDateTime(timestamp);
    }
}
