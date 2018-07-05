using System;
using System.Runtime.InteropServices;
using System.Diagnostics;

static public partial class Yogi
{
    partial class Api
    {
        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate IntPtr GetErrorStringDelegate(int err);
        public static GetErrorStringDelegate YOGI_GetErrorString
            = Library.GetDelegateForFunction<GetErrorStringDelegate>("YOGI_GetErrorString");
    }

    /// <summary>
    /// Error codes returned by functions from the Yogi Core library.
    /// </summary>
    public enum ErrorCode
    {
        Ok = 0,
        Unknown = -1,
        ObjectStillUsed = -2,
        BadAlloc = -3,
        InvalidParam = -4,
        InvalidHandle = -5,
        WrongObjectType = -6,
        Canceled = -7,
        Busy = -8,
        Timeout = -9,
        TimerExpired = -10,
        BufferTooSmall = -11,
        OpenSocketFailed = -12,
        BindSocketFailed = -13,
        ListenSocketFailed = -14,
        SetSocketOptionFailed = -15,
        InvalidRegex = -16,
        OpenFileFailed = -17,
        RwSocketFailed = -18,
        ConnectSocketFailed = -19,
        InvalidMagicPrefix = -20,
        IncompatibleVersion = -21,
        DeserializeMsgFailed = -22,
        AcceptSocketFailed = -23,
        LoopbackConnection = -24,
        PasswordMismatch = -25,
        NetNameMismatch = -26,
        DuplicateBranchName = -27,
        DuplicateBranchPath = -28,
        MessageTooLarge = -29,
        ParsingCmdlineFailed = -30,
        ParsingJsonFailed = -31,
        ParsingFileFailed = -32,
        ConfigNotValid = -33,
        HelpRequested = -34,
        WriteToFileFailed = -35,
        UndefinedVariables = -36,
        NoVariableSupport = -37,
        VariableUsedInKey = -38
    }

    /// <summary>
    /// Represents a result of an operation.
    ///
    /// This is a wrapper around the result code returned by the functions from
    /// the Yogi Core library. A result is represented by a number which is >= 0
    /// in case of success and < 0 in case of a failure.
    /// </summary>
    public class Result
    {
        /// <summary>
        /// Constructor.
        /// </summary>
        /// <param name="value">Number as returned by the Yogi Core library function.</param>
        public Result(int value)
        {
            Value = value;
        }

        /// <summary>
        /// The number as returned by the Yogi Core library function.
        /// </summary>
        public int Value { get; }

        /// <summary>
        /// Error code associated with this result.
        /// </summary>
        public ErrorCode ErrorCode
        {
            get
            {
                return Value < 0 ? (ErrorCode)Value : ErrorCode.Ok;
            }
        }

        /// <summary>
        /// Returns a human-readable string describing the result.
        /// </summary>
        public override string ToString()
        {
            return Marshal.PtrToStringAnsi(Api.YOGI_GetErrorString(Value));
        }

        public static bool operator <(Result lhs, Result rhs)
        {
            return lhs.Value < rhs.Value;
        }

        public static bool operator >(Result lhs, Result rhs)
        {
            return lhs.Value > rhs.Value;
        }

        public static bool operator ==(Result lhs, Result rhs)
        {
            return lhs.Value == rhs.Value;
        }

        public static bool operator !=(Result lhs, Result rhs)
        {
            return lhs.Value != rhs.Value;
        }

        public override bool Equals(object obj)
        {
            if (!(obj is Result)) return false;
            return this == (Result)obj;
        }

        public static bool operator <=(Result lhs, Result rhs)
        {
            return lhs.Value <= rhs.Value;
        }

        public static bool operator >=(Result lhs, Result rhs)
        {
            return lhs.Value >= rhs.Value;
        }

        public static bool operator true(Result res)
        {
            return res.Value >= 0;
        }

        public static bool operator false(Result res)
        {
            return res.Value < 0;
        }

        public static implicit operator bool(Result res)
        {
            return res.Value >= 0;
        }

        public override int GetHashCode()
        {
            return Value;
        }
    }

    /// <summary>
    /// Represents the failure of an operation.
    /// </summary>
    public class Failure : Result
    {
        /// <summary>
        /// Constructor.
        /// </summary>
        /// <param name="value">Number as returned by the Yogi Core library function.</param>
        public Failure(int value)
        : base(value)
        {
            Debug.Assert(value < 0);
        }

        /// <summary>
        /// Constructor.
        /// </summary>
        /// <param name="ec">Associated error code.</param>
        public Failure(ErrorCode ec)
        : this((int)ec)
        {
        }
    }

    /// <summary>
    /// A failure of an operation that includes a description.
    /// </summary>
    public class DescriptiveFailure : Failure
    {
        /// <summary>
        /// Constructor.
        /// </summary>
        /// <param name="value">Number as returned by the Yogi Core library function.</param>
        /// <param name="description">More detailed information about the error.</param>
        /// <returns></returns>
        public DescriptiveFailure(int value, string description)
        : base(value)
        {
            Description = description;
        }

        /// <summary>
        /// More detailed information about the error.
        /// </summary>
        public string Description { get; }

        /// <summary>
        /// Returns a human-readable string describing the error in detail.
        /// </summary>
        public override string ToString()
        {
            return base.ToString() + ". Description: " + Description;
        }
    }

    /// <summary>
    /// Exception wrapping a Failure or DescriptiveFailure object.
    /// </summary>
    public class Exception : System.Exception
    {
        /// <summary>
        /// Constructor.
        /// </summary>
        /// <param name="failure">The wrapped Failure or DescriptiveFailure object.</param>
        public Exception(Failure failure)
        : base(failure.ToString())
        {
            Failure = failure;
        }

        /// <summary>
        /// Constructor.
        /// </summary>
        /// <param name="ec">Error code associated with the failure.</param>
        public Exception(ErrorCode ec)
        : this(new Failure((int)ec))
        {
        }

        /// <summary>
        /// Constructor.
        /// </summary>
        /// <param name="ec">Error code associated with the failure.</param>
        /// <param name="description">Detailed description of the failure.</param>
        public Exception(ErrorCode ec, string description)
        : this(new DescriptiveFailure((int)ec, description))
        {
        }

        /// <summary>
        /// The wrapped Failure or DescriptiveFailure object.
        /// </summary>
        public Failure Failure { get; }
    }

    /// <summary>
    /// Represents the success of an operation.
    /// </summary>
    public class Success : Result
    {
        /// <summary>
        /// Constructor.
        /// </summary>
        /// <param name="value">Number as returned by the Yogi Core library function.</param>
        /// <returns></returns>
        public Success(int value)
        : base(value)
        {
            Debug.Assert(value >= 0);
        }
    }

    static void CheckErrorCode(int result)
    {
        if (result < 0)
        {
            throw new Exception(new Failure(result));
        }
    }

    static Result ErrorCodeToResult(int result)
    {
        if (result < 0)
        {
            return new Failure(result);
        }
        else
        {
            return new Success(result);
        }
    }
}
