using System;
using System.Runtime.InteropServices;

public static partial class Yogi
{
    partial class Api
    {
        // === YOGI_TimerCreate ===
        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate int TimerCreateDelegate(ref IntPtr timer,
            SafeObjectHandle context);

        public static TimerCreateDelegate YOGI_TimerCreate
            = Library.GetDelegateForFunction<TimerCreateDelegate>(
                "YOGI_TimerCreate");

        // === YOGI_TimerStart ===
        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate void TimerStartFnDelegate(int res, IntPtr userarg);

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate int TimerStartDelegate(SafeObjectHandle timer,
            long duration, TimerStartFnDelegate fn, IntPtr userarg);

        public static TimerStartDelegate YOGI_TimerStart
            = Library.GetDelegateForFunction<TimerStartDelegate>(
                "YOGI_TimerStart");

        // === YOGI_TimerCancel ===
        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate int TimerCancelDelegate(SafeObjectHandle timer);

        public static TimerCancelDelegate YOGI_TimerCancel
            = Library.GetDelegateForFunction<TimerCancelDelegate>(
                "YOGI_TimerCancel");
    }

    /// <summary>
    /// Simple timer implementation.
    /// </summary>
    public class Timer : Object
    {
        /// <summary>
        /// Constructor.
        /// </summary>
        /// <param name="context">The context to use.</param>
        public Timer(Context context)
        : base(Create(context), new Object[] {context})
        {
        }

        /// <summary>
        /// Delegate for the start handler function.
        /// </summary>
        /// <param name="res">Result of the wait operation.</param>
        public delegate void HandlerFnDelegate(Result res);

        /// <summary>
        /// Starts the timer.
        ///
        /// If the timer is already running, the timer will be canceled first,
        /// as if Cancel() were called explicitly.
        ///
        /// </summary>
        /// <param name="duration">Time when the timer expires.</param>
        /// <param name="fn">Handler function to call after the given time passed.</param>
        public void Start(Duration duration, HandlerFnDelegate fn)
        {
            if (duration < Duration.Zero)
            {
                throw new ArgumentOutOfRangeException("Duration must not be negative");
            }

            Api.TimerStartFnDelegate wrapper = (ec, userarg) =>
            {
                try
                {
                    fn(ErrorCodeToResult(ec));
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
                int res = Api.YOGI_TimerStart(Handle,
                    duration.IsFinite ? duration.NanosecondsCount : -1, wrapper, wrapperPtr);
                CheckErrorCode(res);
            }
            catch
            {
                wrapperHandle.Free();
                throw;
            }
        }

        /// <summary>
        /// Cancels the timer.
        ///
        /// Canceling the timer will result in the handler function registered via
        /// Start() to be called with a cancellation error.static Note that if the
        /// handler is already scheduled for execution, it will be called without an
        /// error.
        /// </summary>
        /// <returns>True if the timer was canceled successfully and false otherwise
        /// (i.e. the timer has not been started or it already expired).</returns>
        public bool Cancel()
        {
            int res = Api.YOGI_TimerCancel(Handle);
            if (res == (int)ErrorCode.TimerExpired)
            {
                return false;
            }

            CheckErrorCode(res);
            return true;
        }

        static IntPtr Create(Context context)
        {
            var handle = new IntPtr();
            int res = Api.YOGI_TimerCreate(ref handle, context.Handle);
            CheckErrorCode(res);
            return handle;
        }
    }
}
