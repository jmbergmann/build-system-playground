using System;
using System.Runtime.InteropServices;

static public partial class Yogi
{
    static DateTime CoreTimestampToDateTime(long timestamp)
    {
        return new DateTime(timestamp / 100, DateTimeKind.Utc).AddYears(1969);
    }

    static TimeSpan CoreDurationToTimeSpan(long duration)
    {
        return new TimeSpan(duration / 100);
    }
}
