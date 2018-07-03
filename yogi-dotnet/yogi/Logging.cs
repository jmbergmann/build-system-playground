using System;
using System.Runtime.InteropServices;

static public partial class Yogi
{
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
}
