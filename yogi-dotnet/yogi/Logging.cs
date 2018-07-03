using System;
using System.Runtime.InteropServices;

static public partial class Yogi
{
    internal partial class Api
    {
        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate int LogToConsoleDelegate(int verbosity, int stream, int color,
            IntPtr timefmt, IntPtr fmt);
        public static LogToConsoleDelegate YOGI_LogToConsole
            = Library.GetDelegateForFunction<LogToConsoleDelegate>("YOGI_LogToConsole");

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate void LogToHookFnDelegate(int severity, long timestamp, int tid,
            IntPtr comp, IntPtr msg, IntPtr userarg);
        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate int LogToHookDelegate(int verbosity, LogToHookFnDelegate fn,
            IntPtr userarg);
        public static LogToHookDelegate YOGI_LogToHook
            = Library.GetDelegateForFunction<LogToHookDelegate>("YOGI_LogToHook");

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate int LogToFileDelegate(int verbosity, IntPtr filename, IntPtr genFn,
            int genfnsize, IntPtr timefmt, IntPtr fmt);
        public static LogToFileDelegate YOGI_LogToFile
            = Library.GetDelegateForFunction<LogToFileDelegate>("YOGI_LogToFile");

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate int LoggerCreateDelegate(ref IntPtr logger, IntPtr component);
        public static LoggerCreateDelegate YOGI_LoggerCreate
            = Library.GetDelegateForFunction<LoggerCreateDelegate>("YOGI_LoggerCreate");

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate int LoggerGetVerbosityDelegate(IntPtr logger, ref int verbosity);
        public static LoggerGetVerbosityDelegate YOGI_LoggerGetVerbosity
            = Library.GetDelegateForFunction<LoggerGetVerbosityDelegate>(
                "YOGI_LoggerGetVerbosity");

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate int LoggerSetVerbosityDelegate(IntPtr logger, int verbosity);
        public static LoggerSetVerbosityDelegate YOGI_LoggerSetVerbosity
            = Library.GetDelegateForFunction<LoggerSetVerbosityDelegate>(
                "YOGI_LoggerSetVerbosity");

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate int LoggerSetComponentsVerbosityDelegate(IntPtr components, int verbosity,
            ref int count);
        public static LoggerSetComponentsVerbosityDelegate YOGI_LoggerSetComponentsVerbosity
            = Library.GetDelegateForFunction<LoggerSetComponentsVerbosityDelegate>(
                "YOGI_LoggerSetComponentsVerbosity");

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate int LoggerLogDelegate(IntPtr logger, int severity, IntPtr file,
            int line, IntPtr msg);
        public static LoggerLogDelegate YOGI_LoggerLog
            = Library.GetDelegateForFunction<LoggerLogDelegate>("YOGI_LoggerLog");
    }

    /// <summary>
    /// Levels of how verbose logging output is.
    /// </summary>
    public enum Verbosity
    {
        /// <summary>Fatal errors are errors that require a process restart.</summary>
        Fatal = 0,

        /// <summary>Errors that the system can recover from..</summary>
        Error = 1,

        /// <summary>Warnings.</summary>
        Warning = 2,

        /// <summary>Useful general information about the system state.</summary>
        Info = 3,

        /// <summary>Information for debugging.</summary>
        Debug = 4,

        /// <summary>Detailed debugging information.</summary>
        Trace = 5
    }

    /// <summary>
    /// Output streams for writing to the terminal.
    /// </summary>
    public enum Stream
    {
        /// <summary>Standard output.</summary>
        Stdout = 0,

        /// <summary>Standard error output.</summary>
        Stderr = 1
    }

    /// <summary>
    /// Allows the YOGI to write library-internal and user logging to stdout or
    /// stderr.
    ///
    /// Calling this function without any parameters disables logging to the console.
    ///
    /// This function supports colourizing the output if the terminal that the
    /// process is running in supports it. The color used for a log entry depends
    /// on the entry's severity. For example, errors will be printed in red and
    /// warnings in yellow.
    ///
    /// Each log entry contains the *component* tag which describes which part of
    /// a program issued the log entry. For entries created by the library itself,
    /// this parameter is prefixed with the string "Yogi.", followed by the
    /// internal component name. For example, the component tag for a branch would
    /// be "Yogi.Branch".
    ///
    /// The timefmt parameter describes the textual format of a log entry's
    /// timestamp. The following placeholders are supported:
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
    /// The fmt parameter describes the textual format of the complete log entry
    /// as it will appear on the console. The supported placeholders are:
    ///     $t: Timestamp, formatted according to the timefmt parameter.
    ///     $P: Process ID (PID).
    ///     $T: Thread ID.
    ///     $s: Severity as 3 letters (FAT, ERR, WRN, IFO, DBG or TRC).
    ///     $m: Log message.
    ///     $f: Source file name.
    ///     $l: Source line number.
    ///     $c: Component tag.
    ///     $<: Set console color corresponding to severity.
    ///     $>: Reset the colours (also done after each log entry).
    ///     $$: A $ sign.
    /// </summary>
    /// <param name="verbosity">Maximum verbosity of messages to log.</param>
    /// <param name="stream">The stream to use.</param>
    /// <param name="color">Use colours in output.</param>
    /// <param name="timefmt">Format of the timestamp (see above for placeholders).</param>
    /// <param name="fmt">Format of a log entry (see above for placeholders).</param>
    public static void LogToConsole([Optional] Verbosity verbosity, [Optional] Stream stream,
                                    [Optional] bool color, [Optional] string timefmt,
                                    [Optional] string fmt)
    {
    }

    /// <summary>
    /// Delegate for the logging hook callback function.
    /// </summary>
    /// <param name="severity">Severity (verbosity) of the entry.</param>
    /// <param name="timestamp">Time when the entry was created.</param>
    /// <param name="tid">ID of the thread that created the entry.</param>
    /// <param name="file">Source filename.</param>
    /// <param name="line">Source file line number.</param>
    /// <param name="comp">Component that created the entry.</param>
    /// <param name="msg">Log message.</param>
    public delegate void LogToHookFnDelegate(Verbosity severity, DateTime timestamp, int tid,
                                             string file, int line, string comp, string msg);

    /// <summary>
    /// Installs a callback function for receiving log entries.
    ///
    /// This function can be used to get notified whenever the Yogi library itself
    /// or the user produces log messages. These messages can then be processed
    /// further in user code.
    ///
    /// Calling this function without any parameters disables the hook.
    /// </summary>
    /// <param name="verbosity">Maximum verbosity of messages to log.</param>
    /// <param name="fn">Callback function (see above for parameters).</param>
    public static void LogToHook([Optional] Verbosity verbosity,
                                 [Optional] LogToHookFnDelegate fn)
    {
    }

    /// <summary>
    /// Creates log file.
    ///
    /// This function opens a file to write library-internal and user logging
    /// information to. If the file with the given filename already exists then it
    /// will be overwritten.
    ///
    /// Calling this function without any parameters disables logging to a file.
    ///
    /// The filename parameter supports all placeholders that are valid for the
    /// timefmt parameter (see below).
    ///
    /// The timefmt parameter describes the textual format of a log entry's
    /// timestamp. The following placeholders are supported:
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
    /// The fmt parameter describes the textual format of the complete log entry
    /// as it will in the log file. The supported placeholders are:
    ///     $t: Timestamp, formatted according to the timefmt parameter.
    ///     $P: Process ID (PID).
    ///     $T: Thread ID.
    ///     $s: Severity as 3 letters (FAT, ERR, WRN, IFO, DBG or TRC).
    ///     $m: Log message.
    ///     $f: Source file name.
    ///     $l: Source line number.
    ///     $c: Component tag.
    ///     $$: A $ sign.
    /// </summary>
    /// <param name="verbosity">Maximum verbosity of messages to log.</param>
    /// <param name="filename">Path to the log file (see above for placeholders).</param>
    /// <param name="timefmt">Format of the timestamp (see above for placeholders).</param>
    /// <param name="fmt">Format of a log entry (see above for placeholders).</param>
    /// <returns>The generated filename with all placeholders resolved.</returns>
    public static string LogToFile([Optional] Verbosity verbosity, [Optional] string filename,
                                   [Optional] string timefmt, [Optional] string fmt)
    {
        return "";
    }

    /// <summary>
    /// Allows generating log entries.
    ///
    /// A logger is an object used for generating log entries that are tagged with
    /// the logger's component tag. A logger's component tag does not have to be
    /// unique, i.e. multiple loggers can be created using identical component
    /// tags.
    ///
    /// The verbosity of a logger acts as a filter. Only messages with a verbosity
    /// less than or equal to the logger's verbosity are being logged.
    ///
    /// Note: The verbosity of a logger affects only messages logged through that
    ///       particular logger, i.e. if two loggers have identical component tags
    ///       their verbosity settings are still independent from each other.
    /// </summary>
    public class Logger : Object
    {
        /// <summary>
        /// Sets the verbosity of all loggers matching a given component tag.
        ///
        /// This function finds all loggers whose component tag matches the regular
        /// expression given in the components parameter and sets their verbosity
        /// to the value of the verbosity parameter.
        /// </summary>
        /// <param name="components">Regex (ECMAScript) for the component tags to match.</param>
        /// <param name="verbosity">Maximum verbosity entries to be logged.</param>
        /// <returns>Number of matching loggers.</returns>
        public static int SetComponentsVerbosity(string components, Verbosity verbosity)
        {
            return 0;
        }

        /// <summary>
        /// Constructor.
        ///
        /// The verbosity of new loggers is Verbosity.Info by default.
        /// </summary>
        /// <param name="component">The component tag to use.</param>
        public Logger(string component) : base(IntPtr.Zero)
        {
        }

        /// <summary>
        /// Verbosity of the logger.
        /// </summary>
        public Verbosity Verbosity
        {
            get;
            set;
        }

        /// <summary>
        /// Creates a log entry.
        ///
        /// The file and line parameters are optional. By default, they will be
        /// set to the file and line of the calling function.
        /// </summary>
        /// <param name="severity">Severity (verbosity) of the entry.</param>
        /// <param name="msg">Log message.</param>
        /// <param name="file">Source file name.</param>
        /// <param name="line">Source file line number.</param>
        public void Log(Verbosity severity, string msg, [Optional] string file,
                        [Optional] int line)
        {
        }
    }

    /// <summary>
    /// Represents the App logger singleton.
    ///
    /// The App logger always exists and uses "App" as its component tag.
    /// Instances of this class can be created; however, they will always point to
    /// the same logger, i.e. changing its verbosity will change the verbosity of
    /// every AppLogger instance.
    /// </summary>
    public class AppLogger : Logger
    {
        /// <summary>
        /// Constructor.
        /// </summary>
        public AppLogger() : base(null)
        {
        }

        /// <summary>
        /// Returns the class name as a string.
        /// </summary>
        /// <returns></returns>
        public override string ToString()
        {
            return GetType().Name;
        }
    }
}
