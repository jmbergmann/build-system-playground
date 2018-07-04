using System;
using System.Diagnostics;
using System.Runtime.ConstrainedExecution;
using System.Runtime.InteropServices;
using System.Collections.Generic;

static public partial class Yogi
{
    internal partial class Api
    {
        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate int DestroyDelegate(IntPtr handle);
        public static DestroyDelegate YOGI_Destroy
            = Library.GetDelegateForFunction<DestroyDelegate>("YOGI_Destroy");
    }

    public class SafeObjectHandle : SafeHandle
    {
        public SafeObjectHandle(string objectTypeName, IntPtr handle) : base(IntPtr.Zero, true)
        {
            this.ObjectTypeName = objectTypeName;
            this.handle = handle;
        }

        public string ObjectTypeName { get; }

        public override bool IsInvalid
        {
            get { return handle == IntPtr.Zero; }
        }

        [ReliabilityContract(Consistency.WillNotCorruptState, Cer.Success)]
        protected override bool ReleaseHandle()
        {
            Result err = ErrorCodeToResult(Api.YOGI_Destroy(handle));
            if (err)
            {
                string info = "";
                if (err.ErrorCode == ErrorCode.ObjectStillUsed)
                {
                    info = " Check that you don't have circular dependencies on Yogi objects.";
                }

                string s = $"Could not destroy {ObjectTypeName}: {err}.{info}";
                Console.Error.WriteLine(s);

                return false;
            }

            return true;
        }

        public override string ToString()
        {
            return ObjectTypeName + (IsInvalid ? " [INVALID]" : $" {handle.ToInt64(),10:0x}");
        }
    }

    /// <summary>
    /// Base class for all "creatable" objects.
    /// </summary>
    public class Object : IDisposable
    {
        /// <summary>
        /// Constructs the object.
        ///
        /// The objects in the depencies list are stored in order to prevent the
        /// garbage collector to destroy them before this objects gets destroyed.
        /// For example, it would prevent the destruction of the context object in
        /// the following case:
        ///    var timer = new Yogi.Timer(new Yogi.Context())
        /// </summary>
        /// <param name="nativeHandle">Native handle representing the object.</param>
        /// <param name="dependencies">Other objects that this object depends on.</param>
        protected Object(IntPtr nativeHandle, [Optional] Object[] dependencies)
        {
            this.handle = new SafeObjectHandle(GetType().Name, nativeHandle);
            this.dependencies = dependencies;
            Disposed = false;
            refCounter = 1;

            if (dependencies != null)
            {
                foreach (var dependency in dependencies)
                {
                    dependency.IncRefCounter();
                }
            }
        }

        ~Object()
        {
            Dispose(false);
        }

        private void IncRefCounter()
        {
            lock (handle)
            {
                Debug.Assert(refCounter > 0);
                ++refCounter;
            }
        }

        private void DecRefCounter()
        {

            lock (handle)
            {
                Debug.Assert(refCounter > 0);
                --refCounter;
                if (refCounter == 0)
                {
                    handle.Dispose();
                }
            }

            if (dependencies != null)
            {
                foreach (var dependency in dependencies)
                {
                    dependency.DecRefCounter();
                }
            }
        }

        /// <summary>
        /// Disposes the object.
        /// </summary>
        public void Dispose()
        {
            Dispose(true);
            GC.SuppressFinalize(this);
        }

        private void Dispose(bool disposing)
        {
            if (Disposed) return;
            Disposed = true;

            DecRefCounter();
        }

        /// <summary>
        /// Returns a string representation of the object.
        /// </summary>
        /// <returns>String representation of the object.</returns>
        public override string ToString()
        {
            return handle.ToString();
        }

        protected SafeObjectHandle Handle
        {
            get
            {
                if (Disposed)
                {
                    return new SafeObjectHandle(GetType().Name, IntPtr.Zero);
                }
                else
                {
                    return handle;
                }
            }

            private set
            {
                handle = value;
            }
        }

        /// <summary>
        /// Returns true if the Dispose() function has been called for this object.
        /// </summary>
        public bool Disposed { get; private set; }

        private SafeObjectHandle handle;
        private Object[] dependencies;
        private volatile int refCounter;
    }
}
