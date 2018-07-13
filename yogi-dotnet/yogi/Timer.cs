using System;
using System.Runtime.InteropServices;

static public partial class Yogi
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
        public delegate void StartFnDelegate(Result res);

        /// <summary>
        /// Starts the timer.
        ///
        /// If the timer is already running, the timer will be canceled first,
        /// as if Stop() were called explicitly.
        ///
        /// </summary>
        /// <param name="duration">Time when the timer expires.</param>
        /// <param name="fn">Handler function to call after the given time passed.</param>
        public void Start(TimeSpan duration, StartFnDelegate fn)
        {
            Api.TimerStartFnDelegate wrapper = (err, userarg) =>
            {
                try
                {
                    fn(ErrorCodeToResult(err));
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
                int res = Api.YOGI_TimerStart(Handle, TimeSpanToCoreDuration(duration),
                    wrapper, wrapperPtr);
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
